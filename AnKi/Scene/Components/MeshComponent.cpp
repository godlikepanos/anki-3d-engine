// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Core/App.h>

namespace anki {

MeshComponent::MeshComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
}

MeshComponent::~MeshComponent()
{
}

MeshComponent& MeshComponent::setMeshFilename(CString fname)
{
	MeshResourcePtr newRsrc;
	const Error err = ResourceManager::getSingleton().loadResource(fname, newRsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load resource: %s", fname.cstr());
	}
	else
	{
		m_resource = newRsrc;
		m_resourceDirty = true;
	}

	return *this;
}

CString MeshComponent::getMeshFilename() const
{
	return (m_resource) ? m_resource->getFilename() : "*Error*";
}

Bool MeshComponent::isValid() const
{
	return !!m_resource && m_resource->isLoaded();
}

void MeshComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isValid()) [[unlikely]]
	{
		for(GpuSceneArrays::MeshLod::Allocation& alloc : m_gpuSceneMeshLods)
		{
			alloc.free();
		}
		return;
	}

	m_gpuSceneMeshLodsReallocatedThisFrame = false;
	if(!m_resourceDirty) [[likely]]
	{
		return;
	}

	updated = true;
	m_resourceDirty = false;

	const MeshResource& mesh = *m_resource;
	const U32 submeshCount = mesh.getSubMeshCount();

	if(m_gpuSceneMeshLods.getSize() != submeshCount)
	{
		m_gpuSceneMeshLods.destroy();
		m_gpuSceneMeshLods.resize(submeshCount);

		for(GpuSceneArrays::MeshLod::Allocation& a : m_gpuSceneMeshLods)
		{
			a.allocate();
			m_gpuSceneMeshLodsReallocatedThisFrame = true;
		}
	}

	for(U32 submeshIdx = 0; submeshIdx < submeshCount; ++submeshIdx)
	{
		Array<GpuSceneMeshLod, kMaxLodCount> meshLods;

		for(U32 l = 0; l < mesh.getLodCount(); ++l)
		{
			GpuSceneMeshLod& meshLod = meshLods[l];
			meshLod = {};
			meshLod.m_positionScale = mesh.getPositionsScale();
			meshLod.m_positionTranslation = mesh.getPositionsTranslation();

			U32 firstIndex, indexCount, firstMeshlet, meshletCount;
			Aabb aabb;
			mesh.getSubMeshInfo(l, submeshIdx, firstIndex, indexCount, firstMeshlet, meshletCount, aabb);

			U32 totalIndexCount;
			IndexType indexType;
			PtrSize indexUgbOffset;
			mesh.getIndexBufferInfo(l, indexUgbOffset, totalIndexCount, indexType);

			for(VertexStreamId stream = VertexStreamId::kMeshRelatedFirst; stream < VertexStreamId::kMeshRelatedCount; ++stream)
			{
				if(mesh.isVertexStreamPresent(stream))
				{
					U32 vertCount;
					PtrSize ugbOffset;
					mesh.getVertexBufferInfo(l, stream, ugbOffset, vertCount);
					const PtrSize elementSize = getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize;
					ANKI_ASSERT(ugbOffset % elementSize == 0);
					meshLod.m_vertexOffsets[U32(stream)] = U32(ugbOffset / elementSize);
				}
				else
				{
					meshLod.m_vertexOffsets[U32(stream)] = kMaxU32;
				}
			}

			meshLod.m_indexCount = indexCount;

			ANKI_ASSERT(indexUgbOffset % getIndexSize(indexType) == 0);
			meshLod.m_firstIndex = U32(indexUgbOffset / getIndexSize(indexType)) + firstIndex;

			if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders || g_cvarCoreMeshletRendering)
			{
				U32 dummy;
				PtrSize meshletBoundingVolumesUgbOffset, meshletGometryDescriptorsUgbOffset;
				mesh.getMeshletBufferInfo(l, meshletBoundingVolumesUgbOffset, meshletGometryDescriptorsUgbOffset, dummy);

				meshLod.m_firstMeshletBoundingVolume = firstMeshlet + U32(meshletBoundingVolumesUgbOffset / sizeof(MeshletBoundingVolume));
				meshLod.m_firstMeshletGeometryDescriptor = firstMeshlet + U32(meshletGometryDescriptorsUgbOffset / sizeof(MeshletGeometryDescriptor));
				meshLod.m_meshletCount = meshletCount;
			}

			meshLod.m_lod = l;

			if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled)
			{
				const U64 address = mesh.getBottomLevelAccelerationStructure(l, submeshIdx)->getGpuAddress();
				memcpy(&meshLod.m_blasAddress[0], &address, sizeof(meshLod.m_blasAddress));

				meshLod.m_tlasInstanceMask = 0xFFFFFFFF;
			}
		}

		// Copy the last LOD to the rest just in case
		for(U32 l = mesh.getLodCount(); l < kMaxLodCount; ++l)
		{
			meshLods[l] = meshLods[l - 1];
		}

		m_gpuSceneMeshLods[submeshIdx].uploadToGpuScene(meshLods);
	}
}

Error MeshComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);
	return Error::kNone;
}

} // end namespace anki
