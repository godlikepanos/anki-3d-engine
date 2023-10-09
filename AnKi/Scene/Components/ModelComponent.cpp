// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ModelComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>

namespace anki {

ModelComponent::ModelComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_gpuSceneTransforms.allocate();
}

ModelComponent::~ModelComponent()
{
}

void ModelComponent::freeGpuScene()
{
	GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneUniforms);

	for(PatchInfo& patch : m_patchInfos)
	{
		patch.m_gpuSceneMeshLods.free();
		patch.m_gpuSceneRenderable.free();
		patch.m_gpuSceneRenderableAabbDepth.free();
		patch.m_gpuSceneRenderableAabbForward.free();
		patch.m_gpuSceneRenderableAabbGBuffer.free();
		patch.m_gpuSceneRenderableAabbRt.free();

		for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
		{
			RenderStateBucketContainer::getSingleton().removeUser(patch.m_renderStateBucketIndices[t]);
		}
	}
}

void ModelComponent::loadModelResource(CString filename)
{
	ModelResourcePtr rsrc;
	const Error err = ResourceManager::getSingleton().loadResource(filename, rsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load model resource");
		return;
	}

	m_resourceChanged = true;

	m_model = std::move(rsrc);
	const U32 modelPatchCount = m_model->getModelPatches().getSize();

	// Init
	freeGpuScene();
	m_patchInfos.resize(modelPatchCount);
	m_presentRenderingTechniques = RenderingTechniqueBit::kNone;

	// Allocate all uniforms so you can make one allocation
	U32 uniformsSize = 0;
	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		const U32 size = U32(m_model->getModelPatches()[i].getMaterial()->getPrefilledLocalUniforms().getSizeInBytes());
		ANKI_ASSERT((size % 4) == 0);
		uniformsSize += size;
	}

	m_gpuSceneUniforms = GpuSceneBuffer::getSingleton().allocate(uniformsSize, 4);
	uniformsSize = 0;

	// Init the patches
	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		PatchInfo& out = m_patchInfos[i];
		const ModelPatch& in = m_model->getModelPatches()[i];

		out.m_techniques = in.getMaterial()->getRenderingTechniques();
		m_castsShadow = m_castsShadow || in.getMaterial()->castsShadow();
		m_presentRenderingTechniques |= in.getMaterial()->getRenderingTechniques();

		out.m_gpuSceneUniformsOffset = m_gpuSceneUniforms.getOffset() + uniformsSize;
		uniformsSize += U32(in.getMaterial()->getPrefilledLocalUniforms().getSizeInBytes());

		out.m_gpuSceneMeshLods.allocate();
		out.m_gpuSceneRenderable.allocate();

		for(RenderingTechnique t : EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(out.m_techniques))
		{
			switch(t)
			{
			case RenderingTechnique::kGBuffer:
				out.m_gpuSceneRenderableAabbGBuffer.allocate();
				break;
			case RenderingTechnique::kForward:
				out.m_gpuSceneRenderableAabbForward.allocate();
				break;
			case RenderingTechnique::kDepth:
				out.m_gpuSceneRenderableAabbDepth.allocate();
				break;
			case RenderingTechnique::kRtShadow:
				out.m_gpuSceneRenderableAabbRt.allocate();
				break;
			default:
				ANKI_ASSERT(0);
			}
		}
	}
}

Error ModelComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isEnabled()) [[unlikely]]
	{
		updated = false;
		return Error::kNone;
	}

	const Bool resourceUpdated = m_resourceChanged;
	m_resourceChanged = false;
	const Bool moved = info.m_node->movedThisFrame() || m_firstTimeUpdate;
	const Bool movedLastFrame = m_movedLastFrame || m_firstTimeUpdate;
	m_firstTimeUpdate = false;
	m_movedLastFrame = moved;
	const Bool hasSkin = m_skinComponent != nullptr && m_skinComponent->isEnabled();

	updated = resourceUpdated || moved || movedLastFrame;

	// Upload GpuSceneMeshLod, uniforms and GpuSceneRenderable
	if(resourceUpdated) [[unlikely]]
	{
		// Upload the mesh views
		const U32 modelPatchCount = m_model->getModelPatches().getSize();
		for(U32 i = 0; i < modelPatchCount; ++i)
		{
			const ModelPatch& patch = m_model->getModelPatches()[i];
			const MeshResource& mesh = *patch.getMesh();
			const MaterialResource& mtl = *patch.getMaterial();

			Array<GpuSceneMeshLod, kMaxLodCount> meshLods;

			for(U32 l = 0; l < mesh.getLodCount(); ++l)
			{
				GpuSceneMeshLod& meshLod = meshLods[l];
				meshLod = {};
				meshLod.m_positionScale = mesh.getPositionsScale();
				meshLod.m_positionTranslation = mesh.getPositionsTranslation();

				ModelPatchGeometryInfo inf;
				patch.getGeometryInfo(l, inf);

				ANKI_ASSERT((inf.m_indexBufferOffset % getIndexSize(inf.m_indexType)) == 0);
				meshLod.m_firstIndex = U32(inf.m_indexBufferOffset / getIndexSize(inf.m_indexType));
				meshLod.m_indexCount = inf.m_indexCount;

				for(VertexStreamId stream = VertexStreamId::kMeshRelatedFirst; stream < VertexStreamId::kMeshRelatedCount; ++stream)
				{
					if(mesh.isVertexStreamPresent(stream))
					{
						const PtrSize elementSize = getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize;
						ANKI_ASSERT((inf.m_vertexBufferOffsets[stream] % elementSize) == 0);
						meshLod.m_vertexOffsets[U32(stream)] = U32(inf.m_vertexBufferOffsets[stream] / elementSize);
					}
					else
					{
						meshLod.m_vertexOffsets[U32(stream)] = kMaxU32;
					}
				}

				if(inf.m_blas)
				{
					const U64 address = inf.m_blas->getGpuAddress();
					memcpy(&meshLod.m_blasAddress, &address, sizeof(meshLod.m_blasAddress));
					meshLod.m_tlasInstanceMask = 0xFFFFFFFF;
				}
			}

			// Copy the last LOD to the rest just in case
			for(U32 l = mesh.getLodCount(); l < kMaxLodCount; ++l)
			{
				meshLods[l] = meshLods[l - 1];
			}

			m_patchInfos[i].m_gpuSceneMeshLods.uploadToGpuScene(meshLods);

			// Upload the GpuSceneRenderable
			GpuSceneRenderable gpuRenderable = {};
			gpuRenderable.m_worldTransformsOffset = m_gpuSceneTransforms.getGpuSceneOffset();
			gpuRenderable.m_constantsOffset = m_patchInfos[i].m_gpuSceneUniformsOffset;
			gpuRenderable.m_meshLodsOffset = m_patchInfos[i].m_gpuSceneMeshLods.getGpuSceneOffset();
			gpuRenderable.m_boneTransformsOffset = (hasSkin) ? m_skinComponent->getBoneTransformsGpuSceneOffset() : 0;
			if(!!(mtl.getRenderingTechniques() & RenderingTechniqueBit::kRtShadow))
			{
				const RenderingKey key(RenderingTechnique::kRtShadow, 0, false, false);
				const MaterialVariant& variant = mtl.getOrCreateVariant(key);
				gpuRenderable.m_rtShadowsShaderHandleIndex = variant.getRtShaderGroupHandleIndex();
			}
			gpuRenderable.m_uuid = SceneGraph::getSingleton().getNewUuid();
			m_patchInfos[i].m_gpuSceneRenderable.uploadToGpuScene(gpuRenderable);
		}

		// Upload the uniforms
		DynamicArray<U32, MemoryPoolPtrWrapper<StackMemoryPool>> allUniforms(info.m_framePool);
		allUniforms.resize(m_gpuSceneUniforms.getAllocatedSize() / 4);
		U32 count = 0;
		for(U32 i = 0; i < modelPatchCount; ++i)
		{
			const ModelPatch& patch = m_model->getModelPatches()[i];
			const MaterialResource& mtl = *patch.getMaterial();
			memcpy(&allUniforms[count], mtl.getPrefilledLocalUniforms().getBegin(), mtl.getPrefilledLocalUniforms().getSizeInBytes());

			count += U32(mtl.getPrefilledLocalUniforms().getSizeInBytes() / 4);
		}

		ANKI_ASSERT(count * 4 == m_gpuSceneUniforms.getAllocatedSize());
		GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, m_gpuSceneUniforms.getOffset(), m_gpuSceneUniforms.getAllocatedSize(),
													 &allUniforms[0]);
	}

	// Upload transforms
	if(moved || movedLastFrame) [[unlikely]]
	{
		Array<Mat3x4, 2> trfs;
		trfs[0] = Mat3x4(info.m_node->getWorldTransform());
		trfs[1] = Mat3x4(info.m_node->getPreviousWorldTransform());
		m_gpuSceneTransforms.uploadToGpuScene(trfs);
	}

	// Scene bounds update
	const Bool aabbUpdated = moved || resourceUpdated || m_skinComponent;
	if(aabbUpdated) [[unlikely]]
	{
		const Aabb aabbWorld = computeAabbWorldSpace(info.m_node->getWorldTransform());
		SceneGraph::getSingleton().updateSceneBounds(aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz());
	}

	// Update the buckets
	const Bool bucketsNeedUpdate = resourceUpdated || moved != movedLastFrame;
	if(bucketsNeedUpdate)
	{
		const U32 modelPatchCount = m_model->getModelPatches().getSize();
		for(U32 i = 0; i < modelPatchCount; ++i)
		{
			// Refresh the render state buckets
			for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
			{
				RenderStateBucketContainer::getSingleton().removeUser(m_patchInfos[i].m_renderStateBucketIndices[t]);

				if(!(RenderingTechniqueBit(1 << t) & m_patchInfos[i].m_techniques))
				{
					continue;
				}

				// Fill the state
				RenderingKey key;
				key.setLod(0); // Materials don't care
				key.setRenderingTechnique(t);
				key.setSkinned(hasSkin);
				key.setVelocity(moved);

				const MaterialVariant& mvariant = m_model->getModelPatches()[i].getMaterial()->getOrCreateVariant(key);

				RenderStateInfo state;
				state.m_primitiveTopology = PrimitiveTopology::kTriangles;
				state.m_indexedDrawcall = true;
				state.m_program = mvariant.getShaderProgram();

				m_patchInfos[i].m_renderStateBucketIndices[t] = RenderStateBucketContainer::getSingleton().addUser(state, t);
			}
		}
	}

	// Upload the AABBs to the GPU scene
	const Bool gpuSceneAabbsNeedUpdate = aabbUpdated || bucketsNeedUpdate;
	if(gpuSceneAabbsNeedUpdate)
	{
		const Aabb aabbWorld = computeAabbWorldSpace(info.m_node->getWorldTransform());

		const U32 modelPatchCount = m_model->getModelPatches().getSize();
		for(U32 i = 0; i < modelPatchCount; ++i)
		{
			// Do raster techniques
			for(RenderingTechnique t :
				EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(m_patchInfos[i].m_techniques & ~RenderingTechniqueBit::kAllRt))
			{
				const GpuSceneRenderableBoundingVolume gpuVolume = initGpuSceneRenderableBoundingVolume(
					aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz(), m_patchInfos[i].m_gpuSceneRenderable.getIndex(),
					m_patchInfos[i].m_renderStateBucketIndices[t].get());

				switch(t)
				{
				case RenderingTechnique::kGBuffer:
					m_patchInfos[i].m_gpuSceneRenderableAabbGBuffer.uploadToGpuScene(gpuVolume);
					break;
				case RenderingTechnique::kDepth:
					m_patchInfos[i].m_gpuSceneRenderableAabbDepth.uploadToGpuScene(gpuVolume);
					break;
				case RenderingTechnique::kForward:
					m_patchInfos[i].m_gpuSceneRenderableAabbForward.uploadToGpuScene(gpuVolume);
					break;
				default:
					ANKI_ASSERT(0);
				}
			}

			// Do RT techniques
			if(!!(m_patchInfos[i].m_techniques & RenderingTechniqueBit::kAllRt))
			{
				const U32 bucket = 0;
				const GpuSceneRenderableBoundingVolume gpuVolume = initGpuSceneRenderableBoundingVolume(
					aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz(), m_patchInfos[i].m_gpuSceneRenderable.getIndex(), bucket);

				m_patchInfos[i].m_gpuSceneRenderableAabbRt.uploadToGpuScene(gpuVolume);
			}
		}
	}

	return Error::kNone;
}

void ModelComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	ANKI_ASSERT(other);

	if(other->getType() != SceneComponentType::kSkin)
	{
		return;
	}

	const Bool alreadyHasSkinComponent = m_skinComponent != nullptr;
	if(added && !alreadyHasSkinComponent)
	{
		m_skinComponent = static_cast<SkinComponent*>(other);
		m_resourceChanged = true;
	}
	else if(!added && other == m_skinComponent)
	{
		m_skinComponent = nullptr;
		m_resourceChanged = true;
	}
}

Aabb ModelComponent::computeAabbWorldSpace(const Transform& worldTransform) const
{
	Aabb aabbLocal;
	if(m_skinComponent == nullptr) [[likely]]
	{
		aabbLocal = m_model->getBoundingVolume();
	}
	else
	{
		aabbLocal = m_skinComponent->getBoneBoundingVolumeLocalSpace().getCompoundShape(m_model->getBoundingVolume());
	}

	return aabbLocal.getTransformed(worldTransform);
}

} // end namespace anki
