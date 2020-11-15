// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ModelResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/MeshResource.h>
#include <anki/resource/MeshLoader.h>
#include <anki/util/Xml.h>
#include <anki/util/Logger.h>

namespace anki
{

static Bool attributeIsRequired(VertexAttributeLocation loc, Pass pass, Bool hasSkin)
{
	if(pass == Pass::GB || pass == Pass::FS)
	{
		return true;
	}
	else if(!hasSkin)
	{
		return loc == VertexAttributeLocation::POSITION;
	}
	else
	{
		return loc == VertexAttributeLocation::POSITION || loc == VertexAttributeLocation::BONE_INDICES
			   || loc == VertexAttributeLocation::BONE_WEIGHTS;
	}
}

void ModelPatch::getRenderingInfo(const RenderingKey& key, WeakArray<U8> subMeshIndicesArray,
								  ModelRenderingInfo& inf) const
{
	const Bool hasSkin = m_model->getSkeleton().isCreated();

	// Get the resources
	RenderingKey meshKey = key;
	meshKey.setLod(min<U32>(key.getLod(), m_meshCount - 1));
	const MeshResource& mesh = getMesh(meshKey);

	// Get program
	{
		RenderingKey mtlKey = key;
		mtlKey.setLod(min(key.getLod(), m_mtl->getLodCount() - 1));
		mtlKey.setSkinned(hasSkin);

		const MaterialVariant& variant = m_mtl->getOrCreateVariant(mtlKey);

		inf.m_program = variant.getShaderProgram();

		inf.m_boneTransformsBinding = m_mtl->getBoneTransformsBinding();
		inf.m_prevFrameBoneTransformsBinding = m_mtl->getPrevFrameBoneTransformsBinding();
	}

	// Vertex attributes & bindings
	{
		U32 bufferBindingVisitedMask = 0;
		Array<U32, MAX_VERTEX_ATTRIBUTES> realBufferBindingToVirtual;

		inf.m_vertexAttributeCount = 0;
		inf.m_vertexBufferBindingCount = 0;

		for(VertexAttributeLocation loc = VertexAttributeLocation::FIRST; loc < VertexAttributeLocation::COUNT; ++loc)
		{
			if(!mesh.isVertexAttributePresent(loc) || !attributeIsRequired(loc, key.getPass(), hasSkin))
			{
				continue;
			}

			// Attribute
			VertexAttributeInfo& attrib = inf.m_vertexAttributes[inf.m_vertexAttributeCount++];
			attrib.m_location = loc;
			mesh.getVertexAttributeInfo(loc, attrib.m_bufferBinding, attrib.m_format, attrib.m_relativeOffset);

			// Binding. Also, remove any holes in the bindings
			if(!(bufferBindingVisitedMask & (1 << attrib.m_bufferBinding)))
			{
				bufferBindingVisitedMask |= 1 << attrib.m_bufferBinding;

				VertexBufferBinding& binding = inf.m_vertexBufferBindings[inf.m_vertexBufferBindingCount];
				mesh.getVertexBufferInfo(attrib.m_bufferBinding, binding.m_buffer, binding.m_offset, binding.m_stride);

				realBufferBindingToVirtual[attrib.m_bufferBinding] = inf.m_vertexBufferBindingCount;
				++inf.m_vertexBufferBindingCount;
			}

			// Change the binding of the attrib
			attrib.m_bufferBinding = realBufferBindingToVirtual[attrib.m_bufferBinding];
		}

		ANKI_ASSERT(inf.m_vertexAttributeCount != 0 && inf.m_vertexBufferBindingCount != 0);
	}

	// Index buff
	U32 indexCount;
	mesh.getIndexBufferInfo(inf.m_indexBuffer, inf.m_indexBufferOffset, indexCount, inf.m_indexType);

	// Other
	ANKI_ASSERT(subMeshIndicesArray.getSize() == 0 && mesh.getSubMeshCount() == 1 && "Not supported ATM");
	inf.m_drawcallCount = 1;
	inf.m_indicesOffsetArray[0] = 0;
	inf.m_indicesCountArray[0] = indexCount;
}

void ModelPatch::getRayTracingInfo(U32 lod, ModelRayTracingInfo& info) const
{
	ANKI_ASSERT(m_mtl->getSupportedRayTracingTypes() != RayTypeBit::NONE);

	info.m_grObjectReferenceCount = 0;
	memset(&info.m_descriptor, 0, sizeof(info.m_descriptor));

	// Mesh
	const MeshResourcePtr& mesh = m_meshes[min(U32(m_meshCount - 1), lod)];
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

U32 ModelPatch::getLodCount() const
{
	return max<U32>(m_meshCount, getMaterial()->getLodCount());
}

Error ModelPatch::init(ModelResource* model, ConstWeakArray<CString> meshFNames, const CString& mtlFName, Bool async,
					   ResourceManager* manager)
{
	ANKI_ASSERT(meshFNames.getSize() > 0);
	m_model = model;

	// Load material
	ANKI_CHECK(manager->loadResource(mtlFName, m_mtl, async));

	// Load meshes
	m_meshCount = 0;
	for(U32 i = 0; i < meshFNames.getSize(); i++)
	{
		ANKI_CHECK(manager->loadResource(meshFNames[i], m_meshes[i], async));

		// Sanity check
		if(i > 0 && !m_meshes[i]->isCompatible(*m_meshes[i - 1]))
		{
			ANKI_RESOURCE_LOGE("Meshes not compatible");
			return Error::USER_DATA;
		}

		++m_meshCount;
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
		XmlElement materialEl;
		ANKI_CHECK(modelPatchEl.getChildElement("material", materialEl));

		Array<CString, 3> meshesFnames;
		U32 meshesCount = 1;

		// Get mesh
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

		ANKI_CHECK(m_modelPatches[count].init(this, ConstWeakArray<CString>(&meshesFnames[0], meshesCount), cstr, async,
											  &getManager()));

		// Move to next
		ANKI_CHECK(modelPatchEl.getNextSiblingElement("modelPatch", modelPatchEl));
	} while(modelPatchEl);

	// <skeleton>
	XmlElement skeletonEl;
	ANKI_CHECK(rootEl.getChildElementOptional("skeleton", skeletonEl));
	if(skeletonEl)
	{
		CString fname;
		ANKI_CHECK(skeletonEl.getText(fname));
		ANKI_CHECK(getManager().loadResource(fname, m_skeleton));
	}

	// Calculate compound bounding volume
	RenderingKey key;
	key.setLod(0);
	m_visibilityShape = m_modelPatches[0].getMesh(key).getBoundingShape();

	for(auto it = m_modelPatches.getBegin() + 1; it != m_modelPatches.getEnd(); ++it)
	{
		m_visibilityShape = m_visibilityShape.getCompoundShape((*it).getMesh(key).getBoundingShape());
	}

	return Error::NONE;
}

} // end namespace anki
