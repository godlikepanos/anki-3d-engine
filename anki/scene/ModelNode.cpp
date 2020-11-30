// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ModelNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/DebugDrawer.h>
#include <anki/scene/components/BodyComponent.h>
#include <anki/scene/components/SkinComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/resource/ModelResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/SkeletonResource.h>
#include <anki/physics/PhysicsWorld.h>

namespace anki
{

/// Feedback component.
class ModelNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		const SkinComponent* skin = node.tryGetFirstComponentOfType<SkinComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp()
		   || (skin && skin->getTimestamp() == node.getGlobalTimestamp()))
		{
			ModelNode& mnode = static_cast<ModelNode&>(node);
			mnode.updateSpatialComponent(move);
		}

		return Error::NONE;
	}
};

/// Feedback component.
class ModelNode::SkinFeedbackComponent : public SceneComponent
{
public:
	SkinFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const SkinComponent& skin = node.getFirstComponentOfType<SkinComponent>();
		if(skin.getTimestamp() == node.getGlobalTimestamp())
		{
			ModelNode& mnode = static_cast<ModelNode&>(node);

			const Aabb& box = skin.getBoneBoundingVolume();
			mnode.m_obbLocal.setCenter((box.getMin() + box.getMax()) / 2.0f);
			mnode.m_obbLocal.setExtend(box.getMax() - mnode.m_obbLocal.getCenter());
			mnode.m_obbLocal.setRotation(Mat3x4::getIdentity());
		}

		return Error::NONE;
	}
};

ModelNode::ModelNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

ModelNode::~ModelNode()
{
}

Error ModelNode::init(ModelResourcePtr resource, U32 modelPatchIdx)
{
	ANKI_ASSERT(modelPatchIdx < resource->getModelPatches().getSize());

	ANKI_CHECK(m_dbgDrawer.init(&getResourceManager()));
	m_model = resource;
	m_modelPatchIdx = modelPatchIdx;

	// Merge key
	Array<U64, 2> toHash;
	toHash[0] = modelPatchIdx;
	toHash[1] = resource->getUuid();
	m_mergeKey = computeHash(&toHash[0], sizeof(toHash));

	// Components
	if(m_model->getSkeleton().isCreated())
	{
		newComponent<SkinComponent>(this, m_model->getSkeleton());
		newComponent<SkinFeedbackComponent>();
	}
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<SpatialComponent>(this, &m_obbWorld);
	RenderComponent* rcomp = newComponent<RenderComponent>();
	rcomp->initRaster(
		[](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
			const ModelNode& self = *static_cast<const ModelNode*>(userData[0]);
			self.draw(ctx, userData);
		},
		this, m_mergeKey);
	rcomp->setFlagsFromMaterial(m_model->getModelPatches()[m_modelPatchIdx].getMaterial());

	if(m_model->getModelPatches()[m_modelPatchIdx].getSupportedRayTracingTypes() != RayTypeBit::NONE)
	{
		rcomp->initRayTracing(setupRayTracingInstanceQueueElement, this);
	}

	m_obbLocal = m_model->getModelPatches()[m_modelPatchIdx].getBoundingShape();

	return Error::NONE;
}

Error ModelNode::init(const CString& modelFname)
{
	ModelResourcePtr model;
	ANKI_CHECK(getResourceManager().loadResource(modelFname, model));

	// Init this
	ANKI_CHECK(init(model, 0));

	// Create separate nodes for the model patches and make the children
	for(U32 i = 1; i < model->getModelPatches().getSize(); ++i)
	{
		ModelNode* other;
		ANKI_CHECK(getSceneGraph().newSceneNode(CString(), other, model, i));

		addChild(other);
	}

	return Error::NONE;
}

void ModelNode::updateSpatialComponent(const MoveComponent& move)
{
	m_obbWorld = m_obbLocal.getTransformed(move.getWorldTransform());

	SpatialComponent& sp = getFirstComponentOfType<SpatialComponent>();
	sp.markForUpdate();
	sp.setSpatialOrigin(move.getWorldTransform().getOrigin());
}

void ModelNode::draw(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) const
{
	ANKI_ASSERT(userData.getSize() > 0 && userData.getSize() <= MAX_INSTANCES);
	ANKI_ASSERT(ctx.m_key.getInstanceCount() == userData.getSize());

	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(ANKI_LIKELY(!ctx.m_debugDraw))
	{
		const ModelPatch& patch = m_model->getModelPatches()[m_modelPatchIdx];

		// That will not work on multi-draw and instanced at the same time. Make sure that there is no multi-draw
		// anywhere
		ANKI_ASSERT(patch.getSubMeshCount() == 1);

		// Transforms
		Array<Mat4, MAX_INSTANCES> trfs;
		Array<Mat4, MAX_INSTANCES> prevTrfs;
		const MoveComponent& movec = getFirstComponentOfType<MoveComponent>();
		trfs[0] = Mat4(movec.getWorldTransform());
		prevTrfs[0] = Mat4(movec.getPreviousWorldTransform());
		Bool moved = trfs[0] != prevTrfs[0];
		for(U32 i = 1; i < userData.getSize(); ++i)
		{
			const ModelNode& self2 = *static_cast<const ModelNode*>(userData[i]);
			const MoveComponent& movec = self2.getFirstComponentOfType<MoveComponent>();
			trfs[i] = Mat4(movec.getWorldTransform());
			prevTrfs[i] = Mat4(movec.getPreviousWorldTransform());

			moved = moved || (trfs[i] != prevTrfs[i]);
		}

		ctx.m_key.setVelocity(moved && ctx.m_key.getPass() == Pass::GB);
		ModelRenderingInfo modelInf;
		patch.getRenderingInfo(ctx.m_key, WeakArray<U8>(), modelInf);

		// Bones storage
		if(m_model->getSkeleton())
		{
			const SkinComponent& skinc = getComponentAt<SkinComponent>(0);
			const U32 boneCount = skinc.getBoneTransforms().getSize();
			StagingGpuMemoryToken token, tokenPrev;
			void* trfs = ctx.m_stagingGpuAllocator->allocateFrame(boneCount * sizeof(Mat4),
																  StagingGpuMemoryType::STORAGE, token);
			memcpy(trfs, &skinc.getBoneTransforms()[0], boneCount * sizeof(Mat4));

			trfs = ctx.m_stagingGpuAllocator->allocateFrame(boneCount * sizeof(Mat4), StagingGpuMemoryType::STORAGE,
															tokenPrev);
			memcpy(trfs, &skinc.getPreviousFrameBoneTransforms()[0], boneCount * sizeof(Mat4));

			ANKI_ASSERT(modelInf.m_boneTransformsBinding < MAX_U32);
			cmdb->bindStorageBuffer(patch.getMaterial()->getDescriptorSetIndex(), modelInf.m_boneTransformsBinding,
									token.m_buffer, token.m_offset, token.m_range);

			ANKI_ASSERT(modelInf.m_prevFrameBoneTransformsBinding < MAX_U32);
			cmdb->bindStorageBuffer(patch.getMaterial()->getDescriptorSetIndex(),
									modelInf.m_prevFrameBoneTransformsBinding, tokenPrev.m_buffer, tokenPrev.m_offset,
									tokenPrev.m_range);
		}

		// Program
		cmdb->bindShaderProgram(modelInf.m_program);

		// Uniforms
		RenderComponent::allocateAndSetupUniforms(m_model->getModelPatches()[m_modelPatchIdx].getMaterial(), ctx,
												  ConstWeakArray<Mat4>(&trfs[0], userData.getSize()),
												  ConstWeakArray<Mat4>(&prevTrfs[0], userData.getSize()),
												  *ctx.m_stagingGpuAllocator);

		// Set attributes
		for(U i = 0; i < modelInf.m_vertexAttributeCount; ++i)
		{
			const VertexAttributeInfo& attrib = modelInf.m_vertexAttributes[i];
			ANKI_ASSERT(attrib.m_format != Format::NONE);
			cmdb->setVertexAttribute(U32(attrib.m_location), attrib.m_bufferBinding, attrib.m_format,
									 attrib.m_relativeOffset);
		}

		// Set vertex buffers
		for(U32 i = 0; i < modelInf.m_vertexBufferBindingCount; ++i)
		{
			const VertexBufferBinding& binding = modelInf.m_vertexBufferBindings[i];
			cmdb->bindVertexBuffer(i, binding.m_buffer, binding.m_offset, binding.m_stride, VertexStepRate::VERTEX);
		}

		// Index buffer
		cmdb->bindIndexBuffer(modelInf.m_indexBuffer, 0, IndexType::U16);

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, modelInf.m_indicesCountArray[0], userData.getSize(),
						   U32(modelInf.m_indicesOffsetArray[0] / sizeof(U16)), 0, 0);
	}
	else
	{
		// Draw the bounding volumes

		Mat4* const mvps = ctx.m_frameAllocator.newArray<Mat4>(userData.getSize());
		for(U32 i = 0; i < userData.getSize(); ++i)
		{
			const ModelNode& self2 = *static_cast<const ModelNode*>(userData[i]);

			const Mat3 rot = self2.m_obbWorld.getRotation().getRotationPart();
			const Vec4 tsl = self2.m_obbWorld.getCenter().xyz1();
			const Vec3 scale = self2.m_obbWorld.getExtend().xyz();

			// Set non uniform scale. Add a margin to avoid flickering
			Mat3 nonUniScale = Mat3::getZero();
			constexpr F32 MARGIN = 1.02f;
			nonUniScale(0, 0) = scale.x() * MARGIN;
			nonUniScale(1, 1) = scale.y() * MARGIN;
			nonUniScale(2, 2) = scale.z() * MARGIN;

			mvps[i] = ctx.m_viewProjectionMatrix * Mat4(tsl, rot * nonUniScale, 1.0f);
		}

		const Bool enableDepthTest = ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DEPTH_TEST_ON);
		if(enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::LESS);
		}
		else
		{
			cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
		}

		m_dbgDrawer.drawCubes(ConstWeakArray<Mat4>(mvps, userData.getSize()), Vec4(1.0f, 0.0f, 1.0f, 1.0f), 2.0f,
							  ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), 2.0f,
							  *ctx.m_stagingGpuAllocator, cmdb);

		ctx.m_frameAllocator.deleteArray(mvps, userData.getSize());

		// Bones
		if(m_model->getSkeleton())
		{
			const SkinComponent& skinc = getComponentAt<SkinComponent>(0);
			SkeletonResourcePtr skeleton = skinc.getSkeleronResource();
			const U32 boneCount = skinc.getBoneTransforms().getSize();

			DynamicArrayAuto<Vec3> lines(ctx.m_frameAllocator);
			lines.resizeStorage(boneCount * 2);
			DynamicArrayAuto<Vec3> chidlessLines(ctx.m_frameAllocator);
			for(U32 i = 0; i < boneCount; ++i)
			{
				const Bone& bone = skeleton->getBones()[i];
				ANKI_ASSERT(bone.getIndex() == i);
				const Vec4 point(0.0f, 0.0f, 0.0f, 1.0f);
				const Bone* parent = bone.getParent();
				Mat4 m = (parent)
							 ? skinc.getBoneTransforms()[parent->getIndex()] * parent->getVertexTransform().getInverse()
							 : Mat4::getIdentity();
				const Vec3 a = (m * point).xyz();

				m = skinc.getBoneTransforms()[i] * bone.getVertexTransform().getInverse();
				const Vec3 b = (m * point).xyz();

				lines.emplaceBack(a);
				lines.emplaceBack(b);

				if(bone.getChildren().getSize() == 0)
				{
					// If there are not children try to draw something for that bone as well
					chidlessLines.emplaceBack(b);
					const F32 len = (b - a).getLength();
					const Vec3 c = b + (b - a).getNormalized() * len;
					chidlessLines.emplaceBack(c);
				}
			}

			const Mat4 mvp =
				ctx.m_viewProjectionMatrix * Mat4(getFirstComponentOfType<MoveComponent>().getWorldTransform());
			m_dbgDrawer.drawLines(ConstWeakArray<Mat4>(&mvp, 1), Vec4(1.0f), 20.0f,
								  ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON), lines,
								  *ctx.m_stagingGpuAllocator, cmdb);

			m_dbgDrawer.drawLines(ConstWeakArray<Mat4>(&mvp, 1), Vec4(0.7f, 0.7f, 0.7f, 1.0f), 5.0f,
								  ctx.m_debugDrawFlags.get(RenderQueueDebugDrawFlag::DITHERED_DEPTH_TEST_ON),
								  chidlessLines, *ctx.m_stagingGpuAllocator, cmdb);
		}

		// Restore state
		if(!enableDepthTest)
		{
			cmdb->setDepthCompareOperation(CompareOperation::LESS);
		}
	}
}

void ModelNode::setupRayTracingInstanceQueueElement(U32 lod, const void* userData, RayTracingInstanceQueueElement& el)
{
	const ModelNode& self = *static_cast<const ModelNode*>(userData);
	const ModelPatch& patch = self.m_model->getModelPatches()[self.m_modelPatchIdx];

	// printf("%s\n", patch.getMaterial()->getFilename().cstr());

	ModelRayTracingInfo info;
	patch.getRayTracingInfo(lod, info);

	memset(&el, 0, sizeof(el));

	// AS
	el.m_bottomLevelAccelerationStructure = info.m_bottomLevelAccelerationStructure.get();

	// Set the descriptor
	el.m_modelDescriptor = info.m_descriptor;
	const MoveComponent& movec = self.getFirstComponentOfType<MoveComponent>();
	const Mat3x4 worldTrf(movec.getWorldTransform());
	memcpy(&el.m_modelDescriptor.m_worldTransform, &worldTrf, sizeof(worldTrf));
	el.m_modelDescriptor.m_worldRotation = movec.getWorldTransform().getRotation().getRotationPart();

	// Handles
	for(RayType type : EnumIterable<RayType>())
	{
		if(!!(patch.getMaterial()->getSupportedRayTracingTypes() & RayTypeBit(1 << type)))
		{
			el.m_shaderGroupHandleIndices[type] = info.m_shaderGroupHandleIndices[type];
		}
		else
		{
			el.m_shaderGroupHandleIndices[type] = MAX_U32;
		}
	}

	// References
	ANKI_ASSERT(info.m_grObjectReferenceCount <= el.m_grObjects.getSize());
	el.m_grObjectCount = info.m_grObjectReferenceCount;
	for(U32 i = 0; i < info.m_grObjectReferenceCount; ++i)
	{
		el.m_grObjects[i] = info.m_grObjectReferences[i].get();
	}
}

} // end namespace anki
