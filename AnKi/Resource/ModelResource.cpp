// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Util/Logger.h>

namespace anki
{

static Bool attributeIsRequired(VertexAttributeId loc, Pass pass, Bool hasSkin)
{
	if(pass == Pass::GB || pass == Pass::FS)
	{
		return true;
	}
	else if(!hasSkin)
	{
		return loc == VertexAttributeId::POSITION;
	}
	else
	{
		return loc == VertexAttributeId::POSITION || loc == VertexAttributeId::BONE_INDICES
			   || loc == VertexAttributeId::BONE_WEIGHTS;
	}
}

void ModelPatch::getRenderingInfo(const RenderingKey& key, ModelRenderingInfo& inf) const
{
	ANKI_ASSERT(!(!m_model->supportsSkinning() && key.isSkinned()));
	const U32 meshLod = min<U32>(key.getLod(), m_meshLodCount - 1);

	// Vertex attributes & bindings
	{
		U32 bufferBindingVisitedMask = 0;
		Array<U32, MAX_VERTEX_ATTRIBUTES> realBufferBindingToVirtual;

		inf.m_vertexAttributeCount = 0;
		inf.m_vertexBufferBindingCount = 0;

		for(VertexAttributeId loc : EnumIterable<VertexAttributeId>())
		{
			if(!m_presentVertexAttributes.get(loc) || !attributeIsRequired(loc, key.getPass(), key.isSkinned()))
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
				ANKI_ASSERT(outBinding.m_offset != MAX_PTR_SIZE);
				outBinding.m_stride = inBinding.m_stride;
				ANKI_ASSERT(outBinding.m_stride != MAX_PTR_SIZE);

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
	inf.m_indexBufferOffset = 0;
	inf.m_indexCount = m_indexBufferInfos[meshLod].m_indexCount;
	inf.m_firstIndex = m_indexBufferInfos[meshLod].m_firstIndex;
	inf.m_indexType = m_indexType;

	// Get program
	{
		RenderingKey mtlKey = key;
		mtlKey.setLod(min(key.getLod(), m_mtl->getLodCount() - 1));

		const MaterialVariant& variant = m_mtl->getOrCreateVariant(mtlKey);

		inf.m_program = variant.getShaderProgram();

		if(m_mtl->supportsSkinning())
		{
			inf.m_boneTransformsBinding = m_mtl->getBoneTransformsStorageBlockBinding();
			inf.m_prevFrameBoneTransformsBinding = m_mtl->getPrevFrameBoneTransformsStorageBlockBinding();
		}
		else
		{
			inf.m_boneTransformsBinding = inf.m_prevFrameBoneTransformsBinding = MAX_U32;
		}
	}
}

void ModelPatch::getRayTracingInfo(U32 lod, ModelRayTracingInfo& info) const
{
	ANKI_ASSERT(m_mtl->getSupportedRayTracingTypes() != RayTypeBit::NONE);

	info.m_grObjectReferenceCount = 0;
	memset(&info.m_descriptor, 0, sizeof(info.m_descriptor));

	// Mesh
	const MeshResourcePtr& mesh = m_meshes[min(U32(m_meshLodCount - 1), lod)];
	info.m_bottomLevelAccelerationStructure = mesh->getBottomLevelAccelerationStructure();
	info.m_descriptor.m_mesh = mesh->getMeshGpuDescriptor();
	info.m_grObjectReferences[info.m_grObjectReferenceCount++] = mesh->getIndexBuffer();
	info.m_grObjectReferences[info.m_grObjectReferenceCount++] = mesh->getVertexBuffer();

	// Material
	info.m_descriptor.m_material = m_mtl->getMaterialGpuDescriptor();
	for(RayType rayType : EnumIterable<RayType>())
	{
		if(!!(m_mtl->getSupportedRayTracingTypes() & RayTypeBit(1 << rayType)))
		{
			info.m_shaderGroupHandleIndices[rayType] = m_mtl->getShaderGroupHandleIndex(rayType);
		}
		else
		{
			info.m_shaderGroupHandleIndices[rayType] = MAX_U32;
		}
	}

	ConstWeakArray<TextureViewPtr> textureViews = m_mtl->getAllTextureViews();
	for(U32 i = 0; i < textureViews.getSize(); ++i)
	{
		info.m_grObjectReferences[info.m_grObjectReferenceCount++] = textureViews[i];
	}
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

	// Load meshes
	m_meshLodCount = 0;
	for(U32 lod = 0; lod < meshFNames.getSize(); lod++)
	{
		ANKI_CHECK(manager->loadResource(meshFNames[lod], m_meshes[lod], async));

		// Sanity check
		if(lod > 0 && !m_meshes[lod]->isCompatible(*m_meshes[lod - 1]))
		{
			ANKI_RESOURCE_LOGE("Meshes not compatible");
			return Error::USER_DATA;
		}

		// Submesh index
		if(subMeshIndex != MAX_U32 && subMeshIndex >= m_meshes[lod]->getSubMeshCount())
		{
			ANKI_RESOURCE_LOGE("Wrong subMeshIndex given");
			return Error::USER_DATA;
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

			if(subMeshIndex == MAX_U32)
			{
				IndexType indexType;
				PtrSize offset;
				mesh.getIndexBufferInfo(outIndexBufferInfo.m_buffer, offset, outIndexBufferInfo.m_indexCount,
										indexType);
				ANKI_ASSERT(offset == 0);
				m_indexType = indexType;
				outIndexBufferInfo.m_firstIndex = 0;
			}
			else
			{
				IndexType indexType;
				PtrSize offset;
				mesh.getIndexBufferInfo(outIndexBufferInfo.m_buffer, offset, outIndexBufferInfo.m_indexCount,
										indexType);
				ANKI_ASSERT(offset == 0);
				m_indexType = indexType;

				Aabb aabb;
				mesh.getSubMeshInfo(subMeshIndex, outIndexBufferInfo.m_firstIndex, outIndexBufferInfo.m_indexCount,
									aabb);
			}
		}
	}

	return Error::NONE;
}

ModelResource::ModelResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

ModelResource::~ModelResource()
{
	auto alloc = getAllocator();
	m_modelPatches.destroy(alloc);
}

Error ModelResource::load(const ResourceFilename& filename, Bool async)
{
	auto alloc = getAllocator();

	// Load
	//
	XmlElement el;
	XmlDocument doc;
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
		return Error::USER_DATA;
	}

	m_modelPatches.create(alloc, count);

	count = 0;
	ANKI_CHECK(modelPatchesEl.getChildElement("modelPatch", modelPatchEl));
	do
	{
		U32 subMeshIndex;
		Bool subMeshIndexPresent;
		ANKI_CHECK(modelPatchEl.getAttributeNumberOptional("subMeshIndex", subMeshIndex, subMeshIndexPresent));
		if(!subMeshIndexPresent)
		{
			subMeshIndex = MAX_U32;
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
			return Error::USER_DATA;
		}

		m_skinning = m_modelPatches[count].supportsSkinning();

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

	return Error::NONE;
}

} // end namespace anki
