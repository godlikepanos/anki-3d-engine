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

static Bool attributeIsRequired(VertexAttributeId loc, RenderingTechnique technique, Bool hasSkin)
{
	if(technique == RenderingTechnique::kGBuffer || technique == RenderingTechnique::kForward)
	{
		return true;
	}
	else if(!hasSkin)
	{
		return loc == VertexAttributeId::kPosition || loc == VertexAttributeId::kUv0;
	}
	else
	{
		return loc == VertexAttributeId::kPosition || loc == VertexAttributeId::kBoneIndices
			   || loc == VertexAttributeId::kBoneWeights || loc == VertexAttributeId::kUv0;
	}
}

void ModelPatch::getRenderingInfo(const RenderingKey& key, ModelRenderingInfo& inf) const
{
	ANKI_ASSERT(!(!supportsSkinning() && key.getSkinned()));
	const U32 meshLod = min<U32>(key.getLod(), m_meshLodCount - 1);

	// Vertex attributes & bindings
	{
		U32 bufferBindingVisitedMask = 0;
		Array<U32, kMaxVertexAttributes> realBufferBindingToVirtual;

		inf.m_vertexAttributeCount = 0;
		inf.m_vertexBufferBindingCount = 0;

		for(VertexAttributeId loc : EnumIterable<VertexAttributeId>())
		{
			if(!m_presentVertexAttributes.get(loc)
			   || !attributeIsRequired(loc, key.getRenderingTechnique(), key.getSkinned()))
			{
				continue;
			}

			// Attribute
			ModelVertexAttribute& outAttribInfo = inf.m_vertexAttributes[inf.m_vertexAttributeCount++];
			outAttribInfo.m_location = loc;
			outAttribInfo.m_bufferBinding = m_vertexAttributeInfos[loc].m_bufferBinding;
			outAttribInfo.m_relativeOffset = m_vertexAttributeInfos[loc].m_relativeOffset;
			outAttribInfo.m_format = m_vertexAttributeInfos[loc].m_format;

			// Binding. Also, remove any holes in the bindings
			if(!(bufferBindingVisitedMask & (1 << outAttribInfo.m_bufferBinding)))
			{
				bufferBindingVisitedMask |= 1 << outAttribInfo.m_bufferBinding;

				ModelVertexBufferBinding& outBinding = inf.m_vertexBufferBindings[inf.m_vertexBufferBindingCount];
				const VertexBufferInfo& inBinding = m_vertexBufferInfos[meshLod][outAttribInfo.m_bufferBinding];
				outBinding.m_buffer = inBinding.m_buffer;
				ANKI_ASSERT(outBinding.m_buffer.isCreated());
				outBinding.m_offset = inBinding.m_offset;
				ANKI_ASSERT(outBinding.m_offset != kMaxPtrSize);
				outBinding.m_stride = inBinding.m_stride;
				ANKI_ASSERT(outBinding.m_stride != kMaxPtrSize);

				realBufferBindingToVirtual[outAttribInfo.m_bufferBinding] = inf.m_vertexBufferBindingCount;
				++inf.m_vertexBufferBindingCount;
			}

			// Change the binding of the attrib
			outAttribInfo.m_bufferBinding = realBufferBindingToVirtual[outAttribInfo.m_bufferBinding];
		}

		ANKI_ASSERT(inf.m_vertexAttributeCount != 0 && inf.m_vertexBufferBindingCount != 0);
	}

	// Index buff
	inf.m_indexBuffer = m_indexBufferInfos[meshLod].m_buffer;
	inf.m_indexBufferOffset = m_indexBufferInfos[meshLod].m_offset;
	inf.m_indexCount = m_indexBufferInfos[meshLod].m_indexCount;
	inf.m_firstIndex = m_indexBufferInfos[meshLod].m_firstIndex;
	inf.m_indexType = m_indexType;

	// Get program
	const MaterialVariant& variant = m_mtl->getOrCreateVariant(key);
	inf.m_program = variant.getShaderProgram();
}

void ModelPatch::getRayTracingInfo(const RenderingKey& key, ModelRayTracingInfo& info) const
{
	ANKI_ASSERT(!!(m_mtl->getRenderingTechniques() & RenderingTechniqueBit(1 << key.getRenderingTechnique())));

	// Mesh
	const MeshResourcePtr& mesh = m_meshes[min(U32(m_meshLodCount - 1), key.getLod())];
	info.m_bottomLevelAccelerationStructure = mesh->getBottomLevelAccelerationStructure();

	// Material
	const MaterialVariant& variant = m_mtl->getOrCreateVariant(key);
	info.m_shaderGroupHandleIndex = variant.getRtShaderGroupHandleIndex();

	// Misc
	info.m_grObjectReferences = m_grObjectRefs;
}

Error ModelPatch::init(ModelResource* model, ConstWeakArray<CString> meshFNames, const CString& mtlFName,
					   U32 subMeshIndex, Bool async, ResourceManager* manager)
{
	ANKI_ASSERT(meshFNames.getSize() > 0);
#if ANKI_ENABLE_ASSERTIONS
	m_model = model;
#endif

	// Load material
	ANKI_CHECK(manager->loadResource(mtlFName, m_mtl, async));

	// Gather the material refs
	if(m_mtl->getAllTextures().getSize())
	{
		m_grObjectRefs.resizeStorage(model->getMemoryPool(), m_mtl->getAllTextures().getSize());

		for(U32 i = 0; i < m_mtl->getAllTextures().getSize(); ++i)
		{
			m_grObjectRefs.emplaceBack(model->getMemoryPool(), m_mtl->getAllTextures()[i]);
		}
	}

	// Load meshes
	m_meshLodCount = 0;
	for(U32 lod = 0; lod < meshFNames.getSize(); lod++)
	{
		ANKI_CHECK(manager->loadResource(meshFNames[lod], m_meshes[lod], async));

		// Sanity check
		if(lod > 0 && !m_meshes[lod]->isCompatible(*m_meshes[lod - 1]))
		{
			ANKI_RESOURCE_LOGE("Meshes not compatible");
			return Error::kUserData;
		}

		// Submesh index
		if(subMeshIndex != kMaxU32 && subMeshIndex >= m_meshes[lod]->getSubMeshCount())
		{
			ANKI_RESOURCE_LOGE("Wrong subMeshIndex given");
			return Error::kUserData;
		}

		++m_meshLodCount;
	}

	// Create the cached items
	{
		// Vertex attributes
		for(VertexAttributeId attrib : EnumIterable<VertexAttributeId>())
		{
			const MeshResource& mesh = *m_meshes[0].get();

			const Bool enabled = mesh.isVertexAttributePresent(attrib);
			m_presentVertexAttributes.set(U32(attrib), enabled);

			if(!enabled)
			{
				continue;
			}

			VertexAttributeInfo& outAttribInfo = m_vertexAttributeInfos[attrib];
			U32 bufferBinding, relativeOffset;
			mesh.getVertexAttributeInfo(attrib, bufferBinding, outAttribInfo.m_format, relativeOffset);
			outAttribInfo.m_bufferBinding = bufferBinding & 0xFu;
			outAttribInfo.m_relativeOffset = relativeOffset & 0xFFFFFFu;
		}

		// Vertex buffers
		for(U32 lod = 0; lod < m_meshLodCount; ++lod)
		{
			const MeshResource& mesh = *m_meshes[lod].get();

			for(VertexAttributeId attrib : EnumIterable<VertexAttributeId>())
			{
				if(!m_presentVertexAttributes.get(attrib))
				{
					continue;
				}

				VertexBufferInfo& outVertBufferInfo =
					m_vertexBufferInfos[lod][m_vertexAttributeInfos[attrib].m_bufferBinding];
				if(!outVertBufferInfo.m_buffer.isCreated())
				{
					PtrSize offset, stride;
					mesh.getVertexBufferInfo(m_vertexAttributeInfos[attrib].m_bufferBinding, outVertBufferInfo.m_buffer,
											 offset, stride);
					outVertBufferInfo.m_offset = offset & 0xFFFFFFFFFFFF;
					outVertBufferInfo.m_stride = stride & 0xFFFF;
				}
			}
		}

		// Index buffer
		for(U32 lod = 0; lod < m_meshLodCount; ++lod)
		{
			const MeshResource& mesh = *m_meshes[lod].get();
			IndexBufferInfo& outIndexBufferInfo = m_indexBufferInfos[lod];

			if(subMeshIndex == kMaxU32)
			{
				IndexType indexType;
				PtrSize offset;
				mesh.getIndexBufferInfo(outIndexBufferInfo.m_buffer, offset, outIndexBufferInfo.m_indexCount,
										indexType);
				outIndexBufferInfo.m_offset = offset;
				outIndexBufferInfo.m_firstIndex = 0;
				m_indexType = indexType;
			}
			else
			{
				IndexType indexType;
				PtrSize offset;
				mesh.getIndexBufferInfo(outIndexBufferInfo.m_buffer, offset, outIndexBufferInfo.m_indexCount,
										indexType);
				outIndexBufferInfo.m_offset = offset;
				m_indexType = indexType;

				Aabb aabb;
				mesh.getSubMeshInfo(subMeshIndex, outIndexBufferInfo.m_firstIndex, outIndexBufferInfo.m_indexCount,
									aabb);
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
	for(ModelPatch& patch : m_modelPatches)
	{
		patch.m_grObjectRefs.destroy(getMemoryPool());
	}

	m_modelPatches.destroy(getMemoryPool());
}

Error ModelResource::load(const ResourceFilename& filename, Bool async)
{
	HeapMemoryPool& pool = getMemoryPool();

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

	m_modelPatches.create(pool, count);

	count = 0;
	ANKI_CHECK(modelPatchesEl.getChildElement("modelPatch", modelPatchEl));
	do
	{
		U32 subMeshIndex;
		Bool subMeshIndexPresent;
		ANKI_CHECK(modelPatchEl.getAttributeNumberOptional("subMeshIndex", subMeshIndex, subMeshIndexPresent));
		if(!subMeshIndexPresent)
		{
			subMeshIndex = kMaxU32;
		}

		XmlElement materialEl;
		ANKI_CHECK(modelPatchEl.getChildElement("material", materialEl));

		Array<CString, 3> meshesFnames;
		U32 meshesCount = 1;

		XmlElement meshEl;
		ANKI_CHECK(modelPatchEl.getChildElement("mesh", meshEl));

		XmlElement meshEl1;
		ANKI_CHECK(modelPatchEl.getChildElementOptional("mesh1", meshEl1));

		XmlElement meshEl2;
		ANKI_CHECK(modelPatchEl.getChildElementOptional("mesh2", meshEl2));

		ANKI_CHECK(meshEl.getText(meshesFnames[0]));

		if(meshEl1)
		{
			++meshesCount;
			ANKI_CHECK(meshEl1.getText(meshesFnames[1]));
		}

		if(meshEl2)
		{
			++meshesCount;
			ANKI_CHECK(meshEl2.getText(meshesFnames[2]));
		}

		CString cstr;
		ANKI_CHECK(materialEl.getText(cstr));

		ANKI_CHECK(m_modelPatches[count].init(this, ConstWeakArray<CString>(&meshesFnames[0], meshesCount), cstr,
											  subMeshIndex, async, &getManager()));

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
	m_boundingVolume = m_modelPatches[0].m_meshes[0]->getBoundingShape();
	for(auto it = m_modelPatches.getBegin() + 1; it != m_modelPatches.getEnd(); ++it)
	{
		m_boundingVolume = m_boundingVolume.getCompoundShape((*it).m_meshes[0]->getBoundingShape());
	}

	return Error::kNone;
}

} // end namespace anki
