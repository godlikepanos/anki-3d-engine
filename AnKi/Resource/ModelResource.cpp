// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Util/Logger.h>

namespace anki {

void ModelPatch::getRenderingInfo(const RenderingKey& key, ModelRenderingInfo& inf) const
{
	ANKI_ASSERT(!(!supportsSkinning() && key.getSkinned()));
	const U32 meshLod = min<U32>(key.getLod(), m_meshLodCount - 1);

	// Vertex attributes & bindings
	{
		inf.m_indexBufferOffset = m_lodInfos[meshLod].m_indexBufferOffset;
		inf.m_indexType = IndexType::kU16;
		inf.m_firstIndex = m_lodInfos[meshLod].m_firstIndex;
		inf.m_indexCount = m_lodInfos[meshLod].m_indexCount;

		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			inf.m_vertexBufferOffsets[stream] = m_lodInfos[meshLod].m_vertexBufferOffsets[stream];
		}
	}

	// Get program
	const MaterialVariant& variant = m_mtl->getOrCreateVariant(key);
	inf.m_program = variant.getShaderProgram();
}

void ModelPatch::getRayTracingInfo(const RenderingKey& key, ModelRayTracingInfo& info) const
{
	ANKI_ASSERT(!!(m_mtl->getRenderingTechniques() & RenderingTechniqueBit(1 << key.getRenderingTechnique())));

	// Mesh
	const U32 meshLod = min<U32>(key.getLod(), m_meshLodCount - 1);
	info.m_bottomLevelAccelerationStructure = m_mesh->getBottomLevelAccelerationStructure(meshLod);

	// Material
	const MaterialVariant& variant = m_mtl->getOrCreateVariant(key);
	info.m_shaderGroupHandleIndex = variant.getRtShaderGroupHandleIndex();
}

Error ModelPatch::init([[maybe_unused]] ModelResource* model, CString meshFName, const CString& mtlFName,
					   U32 subMeshIndex, Bool async, ResourceManager* manager)
{
#if ANKI_ENABLE_ASSERTIONS
	m_model = model;
#endif

	// Load material
	ANKI_CHECK(manager->loadResource(mtlFName, m_mtl, async));

	// Load mesh
	ANKI_CHECK(manager->loadResource(meshFName, m_mesh, async));

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
		U32 firstIndex, indexCount;
		m_mesh->getSubMeshInfo(0, subMeshIndex, firstIndex, indexCount, m_aabb);
	}

	m_meshLodCount = m_mesh->getLodCount();

	for(U32 l = 0; l < m_meshLodCount; ++l)
	{
		Lod& lod = m_lodInfos[l];
		Aabb aabb;
		m_mesh->getSubMeshInfo(l, (subMeshIndex == kMaxU32) ? 0 : subMeshIndex, lod.m_firstIndex, lod.m_indexCount,
							   aabb);

		U32 totalIndexCount;
		IndexType indexType;
		m_mesh->getIndexBufferInfo(l, lod.m_indexBufferOffset, totalIndexCount, indexType);

		for(VertexStreamId stream : EnumIterable(VertexStreamId::kMeshRelatedFirst, VertexStreamId::kMeshRelatedCount))
		{
			if(m_mesh->isVertexStreamPresent(stream))
			{
				U32 vertCount;
				m_mesh->getVertexStreamInfo(l, stream, lod.m_vertexBufferOffsets[stream], vertCount);
			}
			else
			{
				lod.m_vertexBufferOffsets[stream] = kMaxPtrSize;
			}
		}
	}

	return Error::kNone;
}

ModelResource::ModelResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

ModelResource::~ModelResource()
{
	m_modelPatches.destroy(getMemoryPool());
}

Error ModelResource::load(const ResourceFilename& filename, Bool async)
{
	// Load
	//
	XmlElement el;
	XmlDocument doc(&getTempMemoryPool());
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

	m_modelPatches.create(getMemoryPool(), count);

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

		ANKI_CHECK(m_modelPatches[count].init(this, meshFname, cstr, subMeshIndex, async, &getManager()));

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
