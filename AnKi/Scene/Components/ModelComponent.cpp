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
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_spatial(this)
{
	m_gpuSceneIndexTransforms = GpuSceneContiguousArrays::getSingleton().allocate(GpuSceneContiguousArrayType::kTransformPairs);
}

ModelComponent::~ModelComponent()
{
	m_spatial.removeFromOctree(SceneGraph::getSingleton().getOctree());
}

void ModelComponent::freeGpuScene()
{
	GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneUniforms);

	GpuSceneContiguousArrays& arr = GpuSceneContiguousArrays::getSingleton();

	for(PatchInfo& patch : m_patchInfos)
	{
		arr.deferredFree(patch.m_gpuSceneIndexMeshLods);

		arr.deferredFree(patch.m_gpuSceneIndexRenderable);

		for(GpuSceneContiguousArrayIndex& idx : patch.m_gpuSceneIndexRenderableAabbs)
		{
			arr.deferredFree(idx);
		}

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

	GpuSceneBuffer::getSingleton().allocate(uniformsSize, 4, m_gpuSceneUniforms);
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

		out.m_gpuSceneIndexMeshLods = GpuSceneContiguousArrays::getSingleton().allocate(GpuSceneContiguousArrayType::kMeshLods);

		out.m_gpuSceneIndexRenderable = GpuSceneContiguousArrays::getSingleton().allocate(GpuSceneContiguousArrayType::kRenderables);

		for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
		{
			if(!(RenderingTechniqueBit(1 << t) & out.m_techniques) || !!(RenderingTechniqueBit(1 << t) & RenderingTechniqueBit::kAllRt))
			{
				continue;
			}

			GpuSceneContiguousArrayType allocType = GpuSceneContiguousArrayType::kCount;
			switch(t)
			{
			case RenderingTechnique::kGBuffer:
				allocType = GpuSceneContiguousArrayType::kRenderableBoundingVolumesGBuffer;
				break;
			case RenderingTechnique::kForward:
				allocType = GpuSceneContiguousArrayType::kRenderableBoundingVolumesForward;
				break;
			case RenderingTechnique::kDepth:
				allocType = GpuSceneContiguousArrayType::kRenderableBoundingVolumesDepth;
				break;
			default:
				ANKI_ASSERT(0);
			}

			out.m_gpuSceneIndexRenderableAabbs[t] = GpuSceneContiguousArrays::getSingleton().allocate(allocType);
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

			Array<GpuSceneMeshLod, kMaxLodCount> meshLods;

			for(U32 l = 0; l < mesh.getLodCount(); ++l)
			{
				GpuSceneMeshLod& meshLod = meshLods[l];
				meshLod = {};
				meshLod.m_positionScale = mesh.getPositionsScale();
				meshLod.m_positionTranslation = mesh.getPositionsTranslation();

				for(VertexStreamId stream = VertexStreamId::kPosition; stream <= VertexStreamId::kBoneWeights; ++stream)
				{
					if(!mesh.isVertexStreamPresent(stream))
					{
						continue;
					}

					PtrSize offset;
					U32 vertCount;
					mesh.getVertexStreamInfo(l, stream, offset, vertCount);

					const PtrSize elementSize = getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize;

					ANKI_ASSERT((offset % elementSize) == 0);
					meshLod.m_vertexOffsets[U32(stream)] = U32(offset / elementSize);
				}

				U32 firstIndex;
				U32 indexCount;
				Aabb aabb;
				mesh.getSubMeshInfo(l, i, firstIndex, indexCount, aabb);
				PtrSize offset;
				IndexType indexType;
				mesh.getIndexBufferInfo(l, offset, indexCount, indexType);
				ANKI_ASSERT((U32(offset) % getIndexSize(indexType)) == 0);
				meshLod.m_firstIndex = U32(offset) / getIndexSize(indexType) + firstIndex;
				meshLod.m_indexCount = indexCount;
			}

			// Copy the last LOD to the rest just in case
			for(U32 l = mesh.getLodCount(); l < kMaxLodCount; ++l)
			{
				meshLods[l] = meshLods[l - 1];
			}

			GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, m_patchInfos[i].m_gpuSceneIndexMeshLods.getOffsetInGpuScene(), meshLods);

			// Upload the GpuSceneRenderable
			GpuSceneRenderable gpuRenderable;
			gpuRenderable.m_worldTransformsOffset = m_gpuSceneIndexTransforms.getOffsetInGpuScene();
			gpuRenderable.m_uniformsOffset = m_patchInfos[i].m_gpuSceneUniformsOffset;
			gpuRenderable.m_geometryOffset = m_patchInfos[i].m_gpuSceneIndexMeshLods.getOffsetInGpuScene();
			gpuRenderable.m_boneTransformsOffset = (hasSkin) ? m_skinComponent->getBoneTransformsGpuSceneOffset() : 0;
			GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, m_patchInfos[i].m_gpuSceneIndexRenderable.getOffsetInGpuScene(),
														 gpuRenderable);
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
		GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, m_gpuSceneIndexTransforms.getOffsetInGpuScene(), trfs);
	}

	// Spatial update
	const Bool spatialNeedsUpdate = moved || resourceUpdated || m_skinComponent;
	if(spatialNeedsUpdate) [[unlikely]]
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

		const Aabb aabbWorld = aabbLocal.getTransformed(info.m_node->getWorldTransform());

		m_spatial.setBoundingShape(aabbWorld);
	}

	const Bool spatialUpdated = m_spatial.update(SceneGraph::getSingleton().getOctree());
	updated = updated || spatialUpdated;

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
	const Bool gpuSceneAabbsNeedUpdate = spatialNeedsUpdate || bucketsNeedUpdate;
	if(gpuSceneAabbsNeedUpdate)
	{
		const U32 modelPatchCount = m_model->getModelPatches().getSize();
		for(U32 i = 0; i < modelPatchCount; ++i)
		{
			for(RenderingTechnique t :
				EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(m_patchInfos[i].m_techniques & ~RenderingTechniqueBit::kAllRt))
			{
				const Vec3 aabbMin = m_spatial.getAabbWorldSpace().getMin().xyz();
				const Vec3 aabbMax = m_spatial.getAabbWorldSpace().getMax().xyz();
				const GpuSceneRenderableAabb gpuVolume = initGpuSceneRenderableAabb(aabbMin, aabbMax, m_patchInfos[i].m_gpuSceneIndexRenderable.get(),
																					m_patchInfos[i].m_renderStateBucketIndices[t].get());

				GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool,
															 m_patchInfos[i].m_gpuSceneIndexRenderableAabbs[t].getOffsetInGpuScene(), gpuVolume);
			}
		}
	}

	return Error::kNone;
}

void ModelComponent::setupRenderableQueueElements(U32 lod, RenderingTechnique technique, WeakArray<RenderableQueueElement>& outRenderables) const
{
	ANKI_ASSERT(isEnabled());

	outRenderables.setArray(nullptr, 0);

	const RenderingTechniqueBit requestedRenderingTechniqueMask = RenderingTechniqueBit(1 << technique);
	if(!(m_presentRenderingTechniques & requestedRenderingTechniqueMask))
	{
		return;
	}

	// Allocate renderables
	U32 renderableCount = 0;
	for(U32 i = 0; i < m_patchInfos.getSize(); ++i)
	{
		renderableCount += !!(m_patchInfos[i].m_techniques & requestedRenderingTechniqueMask);
	}

	if(renderableCount == 0)
	{
		return;
	}

	RenderableQueueElement* renderables = static_cast<RenderableQueueElement*>(
		SceneGraph::getSingleton().getFrameMemoryPool().allocate(sizeof(RenderableQueueElement) * renderableCount, alignof(RenderableQueueElement)));

	outRenderables.setArray(renderables, renderableCount);

	// Fill renderables
	const Bool moved = m_node->movedThisFrame() && technique == RenderingTechnique::kGBuffer;
	const Bool hasSkin = m_skinComponent != nullptr && m_skinComponent->isEnabled();

	RenderingKey key;
	key.setLod(lod);
	key.setRenderingTechnique(technique);
	key.setVelocity(moved);
	key.setSkinned(hasSkin);

	renderableCount = 0;
	for(U32 i = 0; i < m_patchInfos.getSize(); ++i)
	{
		if(!(m_patchInfos[i].m_techniques & requestedRenderingTechniqueMask))
		{
			continue;
		}

		RenderableQueueElement& queueElem = renderables[renderableCount];

		const ModelPatch& patch = m_model->getModelPatches()[i];

		ModelRenderingInfo modelInf;
		patch.getRenderingInfo(key, modelInf);

		queueElem.m_program = modelInf.m_program.get();
		queueElem.m_worldTransformsOffset = m_gpuSceneIndexTransforms.getOffsetInGpuScene();
		queueElem.m_uniformsOffset = m_patchInfos[i].m_gpuSceneUniformsOffset;
		queueElem.m_geometryOffset = m_patchInfos[i].m_gpuSceneIndexMeshLods.getOffsetInGpuScene() + lod * sizeof(GpuSceneMeshLod);
		queueElem.m_boneTransformsOffset = (hasSkin) ? m_skinComponent->getBoneTransformsGpuSceneOffset() : 0;
		queueElem.m_indexCount = modelInf.m_indexCount;
		queueElem.m_firstIndex = U32(modelInf.m_indexBufferOffset / 2 + modelInf.m_firstIndex);
		queueElem.m_indexed = true;
		queueElem.m_primitiveTopology = PrimitiveTopology::kTriangles;

		queueElem.m_aabbMin = m_spatial.getAabbWorldSpace().getMin().xyz();
		queueElem.m_aabbMax = m_spatial.getAabbWorldSpace().getMax().xyz();

		queueElem.computeMergeKey();

		++renderableCount;
	}
}

void ModelComponent::setupRayTracingInstanceQueueElements(U32 lod, RenderingTechnique technique,
														  WeakArray<RayTracingInstanceQueueElement>& outInstances) const
{
	ANKI_ASSERT(isEnabled());

	outInstances.setArray(nullptr, 0);

	const RenderingTechniqueBit requestedRenderingTechniqueMask = RenderingTechniqueBit(1 << technique);
	if(!(m_presentRenderingTechniques & requestedRenderingTechniqueMask))
	{
		return;
	}

	// Allocate instances
	U32 instanceCount = 0;
	for(U32 i = 0; i < m_patchInfos.getSize(); ++i)
	{
		instanceCount += !!(m_patchInfos[i].m_techniques & requestedRenderingTechniqueMask);
	}

	if(instanceCount == 0)
	{
		return;
	}

	RayTracingInstanceQueueElement* instances = static_cast<RayTracingInstanceQueueElement*>(SceneGraph::getSingleton().getFrameMemoryPool().allocate(
		sizeof(RayTracingInstanceQueueElement) * instanceCount, alignof(RayTracingInstanceQueueElement)));

	outInstances.setArray(instances, instanceCount);

	RenderingKey key;
	key.setLod(lod);
	key.setRenderingTechnique(technique);

	instanceCount = 0;
	for(U32 i = 0; i < m_patchInfos.getSize(); ++i)
	{
		if(!(m_patchInfos[i].m_techniques & requestedRenderingTechniqueMask))
		{
			continue;
		}

		RayTracingInstanceQueueElement& queueElem = instances[instanceCount];

		const ModelPatch& patch = m_model->getModelPatches()[i];

		GpuSceneContiguousArrays& gpuArrays = GpuSceneContiguousArrays::getSingleton();

		ModelRayTracingInfo modelInf;
		patch.getRayTracingInfo(key, modelInf);

		queueElem.m_bottomLevelAccelerationStructure = modelInf.m_bottomLevelAccelerationStructure.get();
		queueElem.m_shaderGroupHandleIndex = modelInf.m_shaderGroupHandleIndex;
		queueElem.m_worldTransformsOffset = m_gpuSceneIndexTransforms.getOffsetInGpuScene();
		queueElem.m_uniformsOffset = m_patchInfos[i].m_gpuSceneUniformsOffset;
		queueElem.m_geometryOffset =
			U32(m_patchInfos[i].m_gpuSceneIndexMeshLods.get() * sizeof(GpuSceneMeshLod) * kMaxLodCount + lod * sizeof(GpuSceneMeshLod));
		queueElem.m_geometryOffset += U32(gpuArrays.getArrayBase(GpuSceneContiguousArrayType::kMeshLods));
		queueElem.m_indexBufferOffset = U32(modelInf.m_indexBufferOffset);

		const Transform positionTransform(patch.getMesh()->getPositionsTranslation().xyz0(), Mat3x4::getIdentity(),
										  patch.getMesh()->getPositionsScale());
		queueElem.m_transform = Mat3x4(m_node->getWorldTransform()).combineTransformations(Mat3x4(positionTransform));

		++instanceCount;
	}
}

void ModelComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	ANKI_ASSERT(other);

	if(other->getClassId() != SkinComponent::getStaticClassId())
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

} // end namespace anki
