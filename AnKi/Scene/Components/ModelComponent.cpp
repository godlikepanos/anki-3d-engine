// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	getExternalSubsystems(*node).m_gpuSceneMemoryPool->allocate(sizeof(Mat3x4) * 2, alignof(F32), m_gpuSceneTransforms);
}

ModelComponent::~ModelComponent()
{
	GpuSceneMemoryPool& gpuScene = *getExternalSubsystems(*m_node).m_gpuSceneMemoryPool;
	gpuScene.free(m_gpuSceneMeshLods);
	gpuScene.free(m_gpuSceneUniforms);
	gpuScene.free(m_gpuSceneTransforms);

	m_patchInfos.destroy(m_node->getMemoryPool());

	m_spatial.removeFromOctree(m_node->getSceneGraph().getOctree());
}

void ModelComponent::loadModelResource(CString filename)
{
	ModelResourcePtr rsrc;
	const Error err = getExternalSubsystems(*m_node).m_resourceManager->loadResource(filename, rsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load model resource");
		return;
	}

	m_dirty = true;

	m_model = std::move(rsrc);
	const U32 modelPatchCount = m_model->getModelPatches().getSize();

	// GPU scene allocations
	GpuSceneMemoryPool& gpuScene = *getExternalSubsystems(*m_node).m_gpuSceneMemoryPool;

	gpuScene.free(m_gpuSceneMeshLods);
	gpuScene.allocate(sizeof(GpuSceneMeshLod) * kMaxLodCount * modelPatchCount, 4, m_gpuSceneMeshLods);

	U32 uniformsSize = 0;
	m_patchInfos.resize(m_node->getMemoryPool(), modelPatchCount);
	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		m_patchInfos[i].m_gpuSceneUniformsOffset = uniformsSize;

		const U32 size = U32(m_model->getModelPatches()[i].getMaterial()->getPrefilledLocalUniforms().getSizeInBytes());
		ANKI_ASSERT((size % 4) == 0);
		uniformsSize += size;
	}

	gpuScene.free(m_gpuSceneUniforms);
	gpuScene.allocate(uniformsSize, 4, m_gpuSceneUniforms);

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
		GpuSceneMicroPatcher& gpuScenePatcher = *getExternalSubsystems(*info.m_node).m_gpuSceneMicroPatcher;

		// Upload the mesh views
		const U32 modelPatchCount = m_model->getModelPatches().getSize();
		DynamicArrayRaii<GpuSceneMeshLod> meshLods(info.m_framePool, modelPatchCount * kMaxLodCount);
		for(U32 i = 0; i < modelPatchCount; ++i)
		{
			const ModelPatch& patch = m_model->getModelPatches()[i];
			const MeshResource& mesh = *patch.getMesh();

			for(U32 l = 0; l < mesh.getLodCount(); ++l)
			{
				GpuSceneMeshLod& meshLod = meshLods[i * kMaxLodCount + l];
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

				PtrSize offset;
				U32 indexCount;
				IndexType indexType;
				mesh.getIndexBufferInfo(l, offset, indexCount, indexType);
				meshLod.m_indexOffset = U32(offset);
				meshLod.m_indexCount = indexCount;
			}

			// Copy the last LOD to the rest just in case
			for(U32 l = mesh.getLodCount(); l < kMaxLodCount; ++l)
			{
				meshLods[i * kMaxLodCount + l] = meshLods[i * kMaxLodCount + (l - 1)];
			}
		}

		gpuScenePatcher.newCopy(*info.m_framePool, m_gpuSceneMeshLods.m_offset, meshLods.getSizeInBytes(),
								&meshLods[0]);

		// Upload the uniforms
		DynamicArrayRaii<U32> allUniforms(info.m_framePool, U32(m_gpuSceneUniforms.m_size / 4));
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
		gpuScenePatcher.newCopy(*info.m_framePool, m_gpuSceneUniforms.m_offset, m_gpuSceneUniforms.m_size,
								&allUniforms[0]);
	}

	// Upload transforms
	if(moved || movedLastFrame) [[unlikely]]
	{
		Array<Mat3x4, 2> trfs;
		trfs[0] = Mat3x4(info.m_node->getWorldTransform());
		trfs[1] = Mat3x4(info.m_node->getPreviousWorldTransform());

		getExternalSubsystems(*info.m_node)
			.m_gpuSceneMicroPatcher->newCopy(*info.m_framePool, m_gpuSceneTransforms.m_offset, sizeof(trfs), &trfs[0]);
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
			aabbLocal = m_skinComponent->getBoneBoundingVolumeLocalSpace();
		}

		const Aabb aabbWorld = aabbLocal.getTransformed(info.m_node->getWorldTransform());

		m_spatial.setBoundingShape(aabbWorld);
	}

	const Bool spatialUpdated = m_spatial.update(info.m_node->getSceneGraph().getOctree());
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

		queueElem.m_program = modelInf.m_program.get();
		queueElem.m_worldTransformsOffset = U32(m_gpuSceneTransforms.m_offset);
		queueElem.m_uniformsOffset = m_patchInfos[i].m_gpuSceneUniformsOffset;
		queueElem.m_geometryOffset =
			U32(m_gpuSceneMeshLods.m_offset + sizeof(GpuSceneMeshLod) * (kMaxLodCount * i + lod));
		queueElem.m_boneTransformsOffset = (hasSkin) ? m_skinComponent->getBoneTransformsGpuSceneOffset() : 0;
		queueElem.m_indexCount = modelInf.m_indexCount;
		queueElem.m_firstIndex = U32(modelInf.m_indexBufferOffset / 2 + modelInf.m_firstIndex);
		queueElem.m_indexed = true;
		queueElem.m_primitiveTopology = PrimitiveTopology::kTriangles;

		queueElem.computeMergeKey();

		++renderableCount;
	}
}

void ModelComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	ANKI_ASSERT(other);

	if(other->getClassId() != SkinComponent::getStaticClassId())
	{
		return;
	}

	if(added && m_skinComponent != nullptr)
	{
		m_skinComponent = static_cast<SkinComponent*>(other);
		m_dirty = true;
	}
	else if(other == m_skinComponent)
	{
		m_skinComponent = nullptr;
		m_dirty = true;
	}
}

} // end namespace anki
