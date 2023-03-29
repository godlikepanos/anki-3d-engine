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

namespace anki {

ModelComponent::ModelComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_spatial(this)
{
	m_gpuSceneTransformsIndex = U32(SceneGraph::getSingleton().getAllGpuSceneContiguousArrays().allocate(
		GpuSceneContiguousArrayType::kTransformPairs));
}

ModelComponent::~ModelComponent()
{
	GpuSceneMemoryPool::getSingleton().deferredFree(m_gpuSceneUniforms);

	for(const PatchInfo& patch : m_patchInfos)
	{
		if(patch.m_gpuSceneMeshLodsIndex != kMaxU32)
		{
			SceneGraph::getSingleton().getAllGpuSceneContiguousArrays().deferredFree(
				GpuSceneContiguousArrayType::kMeshLods, patch.m_gpuSceneMeshLodsIndex);
		}
	}

	SceneGraph::getSingleton().getAllGpuSceneContiguousArrays().deferredFree(
		GpuSceneContiguousArrayType::kTransformPairs, m_gpuSceneTransformsIndex);

	m_spatial.removeFromOctree(SceneGraph::getSingleton().getOctree());
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

	m_dirty = true;

	m_model = std::move(rsrc);
	const U32 modelPatchCount = m_model->getModelPatches().getSize();

	// GPU scene allocations
	for(const PatchInfo& patch : m_patchInfos)
	{
		if(patch.m_gpuSceneMeshLodsIndex != kMaxU32)
		{
			SceneGraph::getSingleton().getAllGpuSceneContiguousArrays().deferredFree(
				GpuSceneContiguousArrayType::kMeshLods, patch.m_gpuSceneMeshLodsIndex);
		}
	}

	m_patchInfos.resize(modelPatchCount);
	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		m_patchInfos[i].m_gpuSceneMeshLodsIndex =
			U32(SceneGraph::getSingleton().getAllGpuSceneContiguousArrays().allocate(
				GpuSceneContiguousArrayType::kMeshLods));
	}

	U32 uniformsSize = 0;
	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		m_patchInfos[i].m_gpuSceneUniformsOffset = uniformsSize;

		const U32 size = U32(m_model->getModelPatches()[i].getMaterial()->getPrefilledLocalUniforms().getSizeInBytes());
		ANKI_ASSERT((size % 4) == 0);
		uniformsSize += size;
	}

	GpuSceneMemoryPool::getSingleton().deferredFree(m_gpuSceneUniforms);
	GpuSceneMemoryPool::getSingleton().allocate(uniformsSize, 4, m_gpuSceneUniforms);

	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		m_patchInfos[i].m_gpuSceneUniformsOffset += U32(m_gpuSceneUniforms.m_offset);
	}

	// Some other per-patch init
	m_presentRenderingTechniques = RenderingTechniqueBit::kNone;
	m_castsShadow = false;
	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		m_patchInfos[i].m_techniques = m_model->getModelPatches()[i].getMaterial()->getRenderingTechniques();

		m_castsShadow = m_castsShadow || m_model->getModelPatches()[i].getMaterial()->castsShadow();

		m_presentRenderingTechniques |= m_model->getModelPatches()[i].getMaterial()->getRenderingTechniques();
	}
}

Error ModelComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isEnabled()) [[unlikely]]
	{
		updated = false;
		return Error::kNone;
	}

	const Bool resourceUpdated = m_dirty;
	m_dirty = false;
	const Bool moved = info.m_node->movedThisFrame() || m_firstTimeUpdate;
	const Bool movedLastFrame = m_movedLastFrame || m_firstTimeUpdate;
	m_firstTimeUpdate = false;
	m_movedLastFrame = moved;

	updated = resourceUpdated || moved || movedLastFrame;

	// Upload mesh LODs and uniforms
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
				meshLod.m_indexBufferOffset = U32(offset) / getIndexSize(indexType) + firstIndex;
				meshLod.m_indexCount = indexCount;
			}

			// Copy the last LOD to the rest just in case
			for(U32 l = mesh.getLodCount(); l < kMaxLodCount; ++l)
			{
				meshLods[l] = meshLods[l - 1];
			}

			const PtrSize offset = m_patchInfos[i].m_gpuSceneMeshLodsIndex * sizeof(meshLods)
								   + SceneGraph::getSingleton().getAllGpuSceneContiguousArrays().getArrayBase(
									   GpuSceneContiguousArrayType::kMeshLods);
			GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, offset, meshLods.getSizeInBytes(),
														 &meshLods[0]);
		}

		// Upload the uniforms
		DynamicArrayRaii<U32> allUniforms(U32(m_gpuSceneUniforms.m_size / 4), info.m_framePool);
		U32 count = 0;
		for(U32 i = 0; i < modelPatchCount; ++i)
		{
			const ModelPatch& patch = m_model->getModelPatches()[i];
			const MaterialResource& mtl = *patch.getMaterial();
			memcpy(&allUniforms[count], mtl.getPrefilledLocalUniforms().getBegin(),
				   mtl.getPrefilledLocalUniforms().getSizeInBytes());

			count += U32(mtl.getPrefilledLocalUniforms().getSizeInBytes() / 4);
		}

		ANKI_ASSERT(count * 4 == m_gpuSceneUniforms.m_size);
		GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, m_gpuSceneUniforms.m_offset,
													 m_gpuSceneUniforms.m_size, &allUniforms[0]);
	}

	// Upload transforms
	if(moved || movedLastFrame) [[unlikely]]
	{
		Array<Mat3x4, 2> trfs;
		trfs[0] = Mat3x4(info.m_node->getWorldTransform());
		trfs[1] = Mat3x4(info.m_node->getPreviousWorldTransform());

		const PtrSize offset = m_gpuSceneTransformsIndex * sizeof(trfs)
							   + SceneGraph::getSingleton().getAllGpuSceneContiguousArrays().getArrayBase(
								   GpuSceneContiguousArrayType::kTransformPairs);
		GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, offset, sizeof(trfs), &trfs[0]);
	}

	// Spatial update
	if(moved || resourceUpdated || m_skinComponent) [[unlikely]]
	{
		Aabb aabbLocal;
		if(m_skinComponent == nullptr) [[likely]]
		{
			aabbLocal = m_model->getBoundingVolume();
		}
		else
		{
			aabbLocal =
				m_skinComponent->getBoneBoundingVolumeLocalSpace().getCompoundShape(m_model->getBoundingVolume());
		}

		const Aabb aabbWorld = aabbLocal.getTransformed(info.m_node->getWorldTransform());

		m_spatial.setBoundingShape(aabbWorld);
	}

	const Bool spatialUpdated = m_spatial.update(SceneGraph::getSingleton().getOctree());
	updated = updated || spatialUpdated;

	return Error::kNone;
}

void ModelComponent::setupRenderableQueueElements(U32 lod, RenderingTechnique technique, StackMemoryPool& tmpPool,
												  WeakArray<RenderableQueueElement>& outRenderables) const
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
		tmpPool.allocate(sizeof(RenderableQueueElement) * renderableCount, alignof(RenderableQueueElement)));

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

		AllGpuSceneContiguousArrays& gpuArrays = SceneGraph::getSingleton().getAllGpuSceneContiguousArrays();

		queueElem.m_program = modelInf.m_program.get();
		queueElem.m_worldTransformsOffset = U32(m_gpuSceneTransformsIndex * sizeof(Mat3x4) * 2
												+ gpuArrays.getArrayBase(GpuSceneContiguousArrayType::kTransformPairs));
		queueElem.m_uniformsOffset = m_patchInfos[i].m_gpuSceneUniformsOffset;
		queueElem.m_geometryOffset =
			U32(m_patchInfos[i].m_gpuSceneMeshLodsIndex * sizeof(GpuSceneMeshLod) * kMaxLodCount
				+ lod * sizeof(GpuSceneMeshLod));
		queueElem.m_geometryOffset += U32(gpuArrays.getArrayBase(GpuSceneContiguousArrayType::kMeshLods));
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
														  StackMemoryPool& tmpPool,
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

	RayTracingInstanceQueueElement* instances = static_cast<RayTracingInstanceQueueElement*>(tmpPool.allocate(
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

		AllGpuSceneContiguousArrays& gpuArrays = SceneGraph::getSingleton().getAllGpuSceneContiguousArrays();

		ModelRayTracingInfo modelInf;
		patch.getRayTracingInfo(key, modelInf);

		queueElem.m_bottomLevelAccelerationStructure = modelInf.m_bottomLevelAccelerationStructure.get();
		queueElem.m_shaderGroupHandleIndex = modelInf.m_shaderGroupHandleIndex;
		queueElem.m_worldTransformsOffset = U32(m_gpuSceneTransformsIndex * sizeof(Mat3x4) * 2
												+ gpuArrays.getArrayBase(GpuSceneContiguousArrayType::kTransformPairs));
		queueElem.m_uniformsOffset = m_patchInfos[i].m_gpuSceneUniformsOffset;
		queueElem.m_geometryOffset =
			U32(m_patchInfos[i].m_gpuSceneMeshLodsIndex * sizeof(GpuSceneMeshLod) * kMaxLodCount
				+ lod * sizeof(GpuSceneMeshLod));
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
		m_dirty = true;
	}
	else if(!added && other == m_skinComponent)
	{
		m_skinComponent = nullptr;
		m_dirty = true;
	}
}

} // end namespace anki
