// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/Model.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/Material.h>
#include <anki/resource/Mesh.h>
#include <anki/resource/MeshLoader.h>
#include <anki/resource/ShaderResource.h>
#include <anki/misc/Xml.h>
#include <anki/util/Logger.h>

namespace anki
{

ModelPatch::ModelPatch(Model* model)
	: m_model(model)
{
}

ModelPatch::~ModelPatch()
{
}

void ModelPatch::getRenderingDataSub(
	const RenderingKey& key, WeakArray<U8> subMeshIndicesArray, ModelRenderingInfo& inf) const
{
	// Get the resources
	RenderingKey meshKey = key;
	meshKey.m_lod = min<U>(key.m_lod, m_meshCount - 1);
	const Mesh& mesh = getMesh(meshKey);

	// Get program
	{
		RenderingKey mtlKey = key;
		mtlKey.m_lod = min<U>(key.m_lod, m_mtl->getLodCount() - 1);

		const MaterialVariant& variant = m_mtl->getVariant(mtlKey);

		inf.m_program = variant.getShaderProgram();
	}

	// Vertex info
	{
		inf.m_vertexBufferBindingCount = 1;
		VertexBufferBinding& vertBuffBinding = inf.m_vertexBufferBindings[0];
		vertBuffBinding.m_buffer = mesh.getVertexBuffer();
		vertBuffBinding.m_offset = 0;
		vertBuffBinding.m_stride = sizeof(Vec3) + sizeof(HVec2) + 2 * sizeof(U32);

		inf.m_vertexAttributeCount = 4;
		auto& attribs = inf.m_vertexAttributes;

		attribs[0].m_bufferBinding = 0;
		attribs[0].m_format = PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
		attribs[0].m_relativeOffset = 0;

		attribs[1].m_bufferBinding = 0;
		attribs[1].m_format = PixelFormat(ComponentFormat::R16G16, TransformFormat::FLOAT);
		attribs[1].m_relativeOffset = sizeof(Vec3);

		if(key.m_pass == Pass::MS_FS)
		{
			attribs[2].m_bufferBinding = 0;
			attribs[2].m_format = PixelFormat(ComponentFormat::R10G10B10A2, TransformFormat::SNORM);
			attribs[2].m_relativeOffset = sizeof(Vec3) + sizeof(U32);

			attribs[3].m_bufferBinding = 0;
			attribs[3].m_format = PixelFormat(ComponentFormat::R10G10B10A2, TransformFormat::SNORM);
			attribs[3].m_relativeOffset = sizeof(Vec3) + sizeof(U32) * 2;
		}
		else
		{
			inf.m_vertexAttributeCount = 2;
		}
	}

	// Index buff
	inf.m_indexBuffer = mesh.getIndexBuffer();

	// Other
	if(subMeshIndicesArray.getSize() == 0 || mesh.getSubMeshesCount() == 0)
	{
		inf.m_drawcallCount = 1;
		inf.m_indicesOffsetArray[0] = 0;
		inf.m_indicesCountArray[0] = mesh.getIndicesCount();
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}
}

U ModelPatch::getLodCount() const
{
	return max<U>(m_meshCount, getMaterial().getLodCount());
}

Error ModelPatch::create(WeakArray<CString> meshFNames, const CString& mtlFName, ResourceManager* manager)
{
	ANKI_ASSERT(meshFNames.getSize() > 0);

	// Load material
	ANKI_CHECK(manager->loadResource(mtlFName, m_mtl));

	// Load meshes
	m_meshCount = 0;
	for(U i = 0; i < meshFNames.getSize(); i++)
	{
		ANKI_CHECK(manager->loadResource(meshFNames[i], m_meshes[i]));

		// Sanity check
		if(i > 0 && !m_meshes[i]->isCompatible(*m_meshes[i - 1]))
		{
			ANKI_RESOURCE_LOGE("Meshes not compatible");
			return ErrorCode::USER_DATA;
		}

		++m_meshCount;
	}

	return ErrorCode::NONE;
}

Model::Model(ResourceManager* manager)
	: ResourceObject(manager)
{
}

Model::~Model()
{
	auto alloc = getAllocator();

	for(ModelPatch* patch : m_modelPatches)
	{
		alloc.deleteInstance(patch);
	}

	m_modelPatches.destroy(alloc);
}

Error Model::load(const ResourceFilename& filename)
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
	U count = 0;
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
		return ErrorCode::USER_DATA;
	}

	m_modelPatches.create(alloc, count);

	count = 0;
	ANKI_CHECK(modelPatchesEl.getChildElement("modelPatch", modelPatchEl));
	do
	{
		XmlElement materialEl;
		ANKI_CHECK(modelPatchEl.getChildElement("material", materialEl));

		Array<CString, 3> meshesFnames;
		U meshesCount = 1;

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
		ModelPatch* mpatch = alloc.newInstance<ModelPatch>(this);

		if(mpatch == nullptr)
		{
			return ErrorCode::OUT_OF_MEMORY;
		}

		ANKI_CHECK(mpatch->create(WeakArray<CString>(&meshesFnames[0], meshesCount), cstr, &getManager()));

		m_modelPatches[count++] = mpatch;

		// Move to next
		ANKI_CHECK(modelPatchEl.getNextSiblingElement("modelPatch", modelPatchEl));
	} while(modelPatchEl);

	// Calculate compound bounding volume
	RenderingKey key;
	key.m_lod = 0;
	m_visibilityShape = m_modelPatches[0]->getMesh(key).getBoundingShape();

	for(auto it = m_modelPatches.begin() + 1; it != m_modelPatches.end(); ++it)
	{
		m_visibilityShape = m_visibilityShape.getCompoundShape((*it)->getMesh(key).getBoundingShape());
	}

	return ErrorCode::NONE;
}

} // end namespace anki
