// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ModelResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/MaterialResource.h>
#include <anki/resource/MeshResource.h>
#include <anki/resource/MeshLoader.h>
#include <anki/misc/Xml.h>
#include <anki/util/Logger.h>

namespace anki
{

ModelPatch::ModelPatch(ModelResource* model)
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
	const MeshResource& mesh = getMesh(meshKey);

	// Get program
	{
		RenderingKey mtlKey = key;
		mtlKey.m_lod = min<U>(key.m_lod, m_mtl->getLodCount() - 1);

		const MaterialVariant& variant = m_mtl->getOrCreateVariant(mtlKey, m_model->getSkeleton().isCreated());

		inf.m_program = variant.getShaderProgram();
	}

	// Vertex attributes
	U32 positionBinding = MAX_U32;
	{
		if(key.m_pass == Pass::GB_FS)
		{
			// All attributes

			inf.m_vertexAttributeCount = 0;

			for(VertexAttributeLocation loc = VertexAttributeLocation::FIRST; loc < VertexAttributeLocation::COUNT;
				++loc)
			{
				if(!mesh.isVertexAttributePresent(loc))
				{
					continue;
				}

				VertexAttributeInfo& out = inf.m_vertexAttributes[inf.m_vertexAttributeCount++];

				out.m_location = loc;
				mesh.getVertexAttributeInfo(loc, out.m_bufferBinding, out.m_format, out.m_relativeOffset);
			}
		}
		else
		{
			// Only position

			inf.m_vertexAttributeCount = 1;

			VertexAttributeInfo& out = inf.m_vertexAttributes[0];
			out.m_location = VertexAttributeLocation::POSITION;

			mesh.getVertexAttributeInfo(out.m_location, out.m_bufferBinding, out.m_format, out.m_relativeOffset);

			// Rewrite the binding just so we can keep it in a low binding location
			positionBinding = out.m_bufferBinding;
			out.m_bufferBinding = 0;
		}
	}

	// Vertex buffers
	{
		if(key.m_pass == Pass::GB_FS)
		{
			// All attributes

			inf.m_vertexBufferBindingCount = mesh.getVertexBufferCount();

			for(U i = 0; i < inf.m_vertexBufferBindingCount; ++i)
			{
				VertexBufferBinding& out = inf.m_vertexBufferBindings[i];
				mesh.getVertexBufferInfo(i, out.m_buffer, out.m_offset, out.m_stride);
			}
		}
		else
		{
			// Only position

			inf.m_vertexBufferBindingCount = 1;

			VertexBufferBinding& out = inf.m_vertexBufferBindings[0];
			mesh.getVertexBufferInfo(positionBinding, out.m_buffer, out.m_offset, out.m_stride);
		}
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

U ModelPatch::getLodCount() const
{
	return max<U>(m_meshCount, getMaterial()->getLodCount());
}

Error ModelPatch::create(
	ConstWeakArray<CString> meshFNames, const CString& mtlFName, Bool async, ResourceManager* manager)
{
	ANKI_ASSERT(meshFNames.getSize() > 0);

	// Load material
	ANKI_CHECK(manager->loadResource(mtlFName, m_mtl, async));

	// Load meshes
	m_meshCount = 0;
	for(U i = 0; i < meshFNames.getSize(); i++)
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

	for(ModelPatch* patch : m_modelPatches)
	{
		alloc.deleteInstance(patch);
	}

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

		ANKI_CHECK(mpatch->create(ConstWeakArray<CString>(&meshesFnames[0], meshesCount), cstr, async, &getManager()));

		m_modelPatches[count++] = mpatch;

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
	key.m_lod = 0;
	m_visibilityShape = m_modelPatches[0]->getMesh(key).getBoundingShape();

	for(auto it = m_modelPatches.begin() + 1; it != m_modelPatches.end(); ++it)
	{
		m_visibilityShape = m_visibilityShape.getCompoundShape((*it)->getMesh(key).getBoundingShape());
	}

	return Error::NONE;
}

} // end namespace anki
