// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Util/Logger.h>

namespace anki {

// Forward
extern BoolCVar g_meshletRenderingCVar;

void ModelPatch::getGeometryInfo(U32 lod, ModelPatchGeometryInfo& inf) const
{
	lod = min<U32>(lod, m_meshLodCount - 1);

	inf.m_indexUgbOffset = m_lodInfos[lod].m_indexUgbOffset;
	inf.m_indexType = IndexType::kU16;
	inf.m_indexCount = m_lodInfos[lod].m_indexCount;

	for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
	{
		inf.m_vertexUgbOffsets[stream] = m_lodInfos[lod].m_vertexUgbOffsets[stream];
	}

	if(!!(m_mtl->getRenderingTechniques() & RenderingTechniqueBit::kAllRt))
	{
		inf.m_blas = m_mesh->getBottomLevelAccelerationStructure(lod);
	}

	if(m_lodInfos[lod].m_meshletCount != kMaxU32)
	{
		ANKI_ASSERT(m_lodInfos[lod].m_meshletBoundingVolumesUgbOffset != kMaxPtrSize);
		inf.m_meshletCount = m_lodInfos[lod].m_meshletCount;
		inf.m_meshletBoundingVolumesUgbOffset = m_lodInfos[lod].m_meshletBoundingVolumesUgbOffset;
		inf.m_meshletGometryDescriptorsUgbOffset = m_lodInfos[lod].m_meshletGometryDescriptorsUgbOffset;
	}
	else
	{
		inf.m_meshletCount = 0;
		inf.m_meshletBoundingVolumesUgbOffset = kMaxPtrSize;
		inf.m_meshletGometryDescriptorsUgbOffset = kMaxPtrSize;
	}
}

void ModelPatch::getRayTracingInfo(const RenderingKey& key, ModelRayTracingInfo& info) const
{
	ANKI_ASSERT(!!(m_mtl->getRenderingTechniques() & RenderingTechniqueBit(1 << key.getRenderingTechnique())));

	// Mesh
	const U32 meshLod = min<U32>(key.getLod(), m_meshLodCount - 1);
	info.m_bottomLevelAccelerationStructure = m_mesh->getBottomLevelAccelerationStructure(meshLod);

	info.m_indexUgbOffset = m_lodInfos[meshLod].m_indexUgbOffset;

	// Material
	const MaterialVariant& variant = m_mtl->getOrCreateVariant(key);
	info.m_shaderGroupHandleIndex = variant.getRtShaderGroupHandleIndex();
}

Error ModelPatch::init([[maybe_unused]] ModelResource* model, CString meshFName, const CString& mtlFName, U32 subMeshIndex, Bool async)
{
#if ANKI_ASSERTIONS_ENABLED
	m_model = model;
#endif

	// Load material
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(mtlFName, m_mtl, async));

	// Load mesh
	ANKI_CHECK(ResourceManager::getSingleton().loadResource(meshFName, m_mesh, async));

	if(subMeshIndex != kMaxU32 && subMeshIndex >= m_mesh->getSubMeshCount())
	{
		ANKI_RESOURCE_LOGE("Wrong subMeshIndex given");
		return Error::kUserData;
	}

	// Init cached data
	if(subMeshIndex == kMaxU32)
	{
		m_aabb = m_mesh->getBoundingShape();
	}
	else
	{
		U32 firstIndex, indexCount, firstMeshlet, meshletCount;
		m_mesh->getSubMeshInfo(0, subMeshIndex, firstIndex, indexCount, firstMeshlet, meshletCount, m_aabb);
	}

	m_meshLodCount = m_mesh->getLodCount();

	for(U32 l = 0; l < m_meshLodCount; ++l)
	{
		Lod& lod = m_lodInfos[l];
		Aabb aabb;
		U32 firstIndex, firstMeshlet, meshletCount;
		m_mesh->getSubMeshInfo(l, (subMeshIndex == kMaxU32) ? 0 : subMeshIndex, firstIndex, lod.m_indexCount, firstMeshlet, meshletCount, aabb);

		U32 totalIndexCount;
		IndexType indexType;
		m_mesh->getIndexBufferInfo(l, lod.m_indexUgbOffset, totalIndexCount, indexType);
		lod.m_indexUgbOffset += firstIndex * getIndexSize(indexType);

		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			if(m_mesh->isVertexStreamPresent(stream))
			{
				U32 vertCount;
				m_mesh->getVertexBufferInfo(l, stream, lod.m_vertexUgbOffsets[stream], vertCount);
			}
			else
			{
				lod.m_vertexUgbOffsets[stream] = kMaxPtrSize;
			}
		}

		if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders || g_meshletRenderingCVar.get())
		{
			U32 dummy;
			m_mesh->getMeshletBufferInfo(l, lod.m_meshletBoundingVolumesUgbOffset, lod.m_meshletGometryDescriptorsUgbOffset, dummy);

			lod.m_meshletBoundingVolumesUgbOffset += firstMeshlet * sizeof(MeshletBoundingVolume);
			lod.m_meshletGometryDescriptorsUgbOffset += firstMeshlet * sizeof(MeshletGeometryDescriptor);
			lod.m_meshletCount = meshletCount;
		}
	}

	return Error::kNone;
}

Error ModelResource::load(const ResourceFilename& filename, Bool async)
{
	// Load
	//
	ResourceXmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("model", rootEl));

	// <modelPatches>
	XmlElement modelPatchesEl;
	ANKI_CHECK(rootEl.getChildElement("modelPatches", modelPatchesEl));

	XmlElement modelPatchEl;
	ANKI_CHECK(modelPatchesEl.getChildElement("modelPatch", modelPatchEl));

	// Count
	U32 count = 0;
	do
	{
		++count;
		// Move to next
		ANKI_CHECK(modelPatchEl.getNextSiblingElement("modelPatch", modelPatchEl));
	} while(modelPatchEl);

	// Check number of model patches
	if(count < 1)
	{
		ANKI_RESOURCE_LOGE("Zero number of model patches");
		return Error::kUserData;
	}

	m_modelPatches.resize(count);

	count = 0;
	ANKI_CHECK(modelPatchesEl.getChildElement("modelPatch", modelPatchEl));
	do
	{
		XmlElement materialEl;
		ANKI_CHECK(modelPatchEl.getChildElement("material", materialEl));

		XmlElement meshEl;
		ANKI_CHECK(modelPatchEl.getChildElement("mesh", meshEl));
		CString meshFname;
		ANKI_CHECK(meshEl.getText(meshFname));

		U32 subMeshIndex;
		Bool subMeshIndexPresent;
		ANKI_CHECK(meshEl.getAttributeNumberOptional("subMeshIndex", subMeshIndex, subMeshIndexPresent));
		if(!subMeshIndexPresent)
		{
			subMeshIndex = kMaxU32;
		}

		CString cstr;
		ANKI_CHECK(materialEl.getText(cstr));

		ANKI_CHECK(m_modelPatches[count].init(this, meshFname, cstr, subMeshIndex, async));

		if(count > 0 && m_modelPatches[count].supportsSkinning() != m_modelPatches[count - 1].supportsSkinning())
		{
			ANKI_RESOURCE_LOGE("All model patches should support skinning or all shouldn't support skinning");
			return Error::kUserData;
		}

		// Move to next
		ANKI_CHECK(modelPatchEl.getNextSiblingElement("modelPatch", modelPatchEl));
		++count;
	} while(modelPatchEl);
	ANKI_ASSERT(count == m_modelPatches.getSize());

	// Calculate compound bounding volume
	m_boundingVolume = m_modelPatches[0].m_aabb;
	for(auto it = m_modelPatches.getBegin() + 1; it != m_modelPatches.getEnd(); ++it)
	{
		m_boundingVolume = m_boundingVolume.getCompoundShape((*it).m_aabb);
	}

	return Error::kNone;
}

} // end namespace anki
