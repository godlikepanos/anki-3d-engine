// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ModelComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(ModelComponent)

ModelComponent::ModelComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
}

ModelComponent::~ModelComponent()
{
	m_modelPatchMergeKeys.destroy(m_node->getMemoryPool());

	GpuSceneMemoryPool& gpuScene = *getExternalSubsystems(*m_node).m_gpuSceneMemoryPool;
	gpuScene.free(m_gpuSceneMeshLods);
	gpuScene.free(m_gpuSceneUniforms);

	m_gpuSceneUniformsOffsetPerPatch.destroy(m_node->getMemoryPool());
}

Error ModelComponent::loadModelResource(CString filename)
{
	m_dirty = true;

	ModelResourcePtr rsrc;
	ANKI_CHECK(getExternalSubsystems(*m_node).m_resourceManager->loadResource(filename, rsrc));
	m_model = std::move(rsrc);
	const U32 modelPatchCount = m_model->getModelPatches().getSize();

	m_modelPatchMergeKeys.destroy(m_node->getMemoryPool());
	m_modelPatchMergeKeys.create(m_node->getMemoryPool(), modelPatchCount);

	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		Array<U64, 2> toHash;
		toHash[0] = i;
		toHash[1] = m_model->getUuid();
		m_modelPatchMergeKeys[i] = computeHash(&toHash[0], sizeof(toHash));
	}

	// GPU scene allocations
	GpuSceneMemoryPool& gpuScene = *getExternalSubsystems(*m_node).m_gpuSceneMemoryPool;

	gpuScene.free(m_gpuSceneMeshLods);
	gpuScene.allocate(sizeof(GpuSceneMeshLod) * kMaxLodCount * m_modelPatchMergeKeys.getSize(), 4, m_gpuSceneMeshLods);

	U32 uniformsSize = 0;
	m_gpuSceneUniformsOffsetPerPatch.resize(m_node->getMemoryPool(), modelPatchCount);
	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		m_gpuSceneUniformsOffsetPerPatch[i] = uniformsSize;

		const U32 size = U32(m_model->getModelPatches()[i].getMaterial()->getPrefilledLocalUniforms().getSizeInBytes());
		ANKI_ASSERT((size % 4) == 0);
		uniformsSize += size;
	}

	gpuScene.free(m_gpuSceneUniforms);
	gpuScene.allocate(uniformsSize, 4, m_gpuSceneUniforms);

	for(U32 i = 0; i < modelPatchCount; ++i)
	{
		m_gpuSceneUniformsOffsetPerPatch[i] += U32(m_gpuSceneUniforms.m_offset);
	}

	return Error::kNone;
}

Error ModelComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(ANKI_UNLIKELY(m_dirty && m_model.isCreated()))
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

	updated = m_dirty;
	m_dirty = false;
	return Error::kNone;
}

} // end namespace anki
