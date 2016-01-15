// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
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
#include <anki/renderer/Ms.h>
#include <anki/renderer/Is.h>
#include <anki/renderer/Sm.h>

namespace anki
{

//==============================================================================
// ModelPatch                                                                  =
//==============================================================================

//==============================================================================
ModelPatch::ModelPatch(Model* model)
	: m_model(model)
{
}

//==============================================================================
ModelPatch::~ModelPatch()
{
}

//==============================================================================
void ModelPatch::getRenderingDataSub(const RenderingKey& key,
	SArray<U8> subMeshIndicesArray,
	ResourceGroupPtr& resourceGroup,
	PipelinePtr& ppline,
	Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesCountArray,
	Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesOffsetArray,
	U32& drawcallCount) const
{
	// Get the resources
	RenderingKey meshKey = key;
	meshKey.m_lod = min<U>(key.m_lod, m_meshCount - 1);
	const Mesh& mesh = getMesh(meshKey);
	resourceGroup = m_grResources[meshKey.m_lod];

	// Get ppline
	RenderingKey mtlKey = key;
	mtlKey.m_lod = min<U>(key.m_lod, m_mtl->getLodCount() - 1);

	ppline = getPipeline(mtlKey);

	if(subMeshIndicesArray.getSize() == 0 || mesh.getSubMeshesCount() == 0)
	{
		drawcallCount = 1;
		indicesOffsetArray[0] = 0;
		indicesCountArray[0] = mesh.getIndicesCount();
	}
	else
	{
		ANKI_ASSERT(0 && "TODO");
	}
}

//==============================================================================
U ModelPatch::getLodCount() const
{
	return max<U>(m_meshCount, getMaterial().getLodCount());
}

//==============================================================================
Error ModelPatch::create(SArray<CString> meshFNames,
	const CString& mtlFName,
	ResourceManager* manager)
{
	ANKI_ASSERT(meshFNames.getSize() > 0);

	// Load material
	ANKI_CHECK(manager->loadResource(mtlFName, m_mtl));

	// Iterate material variables for textures
	ResourceGroupInitializer rcinit;
	m_mtl->fillResourceGroupInitializer(rcinit);

	// Load meshes and update resource group
	m_meshCount = 0;
	for(U i = 0; i < meshFNames.getSize(); i++)
	{
		ANKI_CHECK(manager->loadResource(meshFNames[i], m_meshes[i]));

		// Sanity check
		if(i > 0 && !m_meshes[i]->isCompatible(*m_meshes[i - 1]))
		{
			ANKI_LOGE("Meshes not compatible");
			return ErrorCode::USER_DATA;
		}

		rcinit.m_vertexBuffers[0].m_buffer = m_meshes[i]->getVertexBuffer();
		rcinit.m_indexBuffer.m_buffer = m_meshes[i]->getIndexBuffer();
		rcinit.m_indexSize = 2;

		m_grResources[i] =
			manager->getGrManager().newInstance<ResourceGroup>(rcinit);

		++m_meshCount;
	}

	return ErrorCode::NONE;
}

//==============================================================================
PipelinePtr ModelPatch::getPipeline(const RenderingKey& key) const
{
	// Preconditions
	ANKI_ASSERT(key.m_lod < m_mtl->getLodCount());

	if(key.m_tessellation && !m_mtl->getTessellationEnabled())
	{
		ANKI_ASSERT(0);
	}

	if(key.m_pass == Pass::SM && !m_mtl->getShadowEnabled())
	{
		ANKI_ASSERT(0);
	}

	if(key.m_instanceCount > 1 && !m_mtl->isInstanced())
	{
		ANKI_ASSERT(0);
	}

	LockGuard<SpinLock> lock(m_lock);

	PipelinePtr& ppline =
		m_pplines[U(key.m_pass)][key.m_lod][key.m_tessellation]
				 [Material::getInstanceGroupIdx(key.m_instanceCount)];

	// Lazily create it
	if(ANKI_UNLIKELY(!ppline.isCreated()))
	{
		const MaterialVariant& variant = m_mtl->getVariant(key);

		PipelineInitializer pplineInit;
		computePipelineInitializer(key, pplineInit);

		pplineInit.m_shaders[U(ShaderType::VERTEX)] =
			variant.getShader(ShaderType::VERTEX);

		if(key.m_tessellation)
		{
			pplineInit.m_shaders[U(ShaderType::TESSELLATION_CONTROL)] =
				variant.getShader(ShaderType::TESSELLATION_CONTROL);

			pplineInit.m_shaders[U(ShaderType::TESSELLATION_EVALUATION)] =
				variant.getShader(ShaderType::TESSELLATION_EVALUATION);
		}

		pplineInit.m_shaders[U(ShaderType::FRAGMENT)] =
			variant.getShader(ShaderType::FRAGMENT);

		// Create
		ppline = m_model->getManager().getGrManager().newInstance<Pipeline>(
			pplineInit);
	}

	return ppline;
}

//==============================================================================
void ModelPatch::computePipelineInitializer(
	const RenderingKey& key, PipelineInitializer& pinit) const
{
	//
	// Vertex state
	//
	VertexStateInfo& vert = pinit.m_vertex;
	if(m_meshes[0]->hasBoneWeights())
	{
		ANKI_ASSERT(0 && "TODO");
	}
	else
	{
		vert.m_bindingCount = 1;
		vert.m_attributeCount = 4;
		vert.m_bindings[0].m_stride =
			sizeof(Vec3) + sizeof(HVec2) + 2 * sizeof(U32);

		vert.m_attributes[0].m_format =
			PixelFormat(ComponentFormat::R32G32B32, TransformFormat::FLOAT);
		vert.m_attributes[0].m_offset = 0;

		vert.m_attributes[1].m_format =
			PixelFormat(ComponentFormat::R16G16, TransformFormat::FLOAT);
		vert.m_attributes[1].m_offset = sizeof(Vec3);

		if(key.m_pass == Pass::MS_FS)
		{
			vert.m_attributes[2].m_format = PixelFormat(
				ComponentFormat::R10G10B10A2, TransformFormat::SNORM);
			vert.m_attributes[2].m_offset = sizeof(Vec3) + sizeof(U32);

			vert.m_attributes[3].m_format = PixelFormat(
				ComponentFormat::R10G10B10A2, TransformFormat::SNORM);
			vert.m_attributes[3].m_offset = sizeof(Vec3) + sizeof(U32) * 2;
		}
		else
		{
			vert.m_attributeCount = 2;
		}
	}

	//
	// Depth/stencil state
	//
	DepthStencilStateInfo& ds = pinit.m_depthStencil;
	if(key.m_pass == Pass::SM)
	{
		ds.m_format = Sm::DEPTH_RT_PIXEL_FORMAT;
		ds.m_polygonOffsetFactor = 1.0;
		ds.m_polygonOffsetUnits = 2.0;
	}
	else if(m_mtl->getForwardShading())
	{
		ds.m_format = Ms::DEPTH_RT_PIXEL_FORMAT;
		ds.m_depthWriteEnabled = false;
	}
	else
	{
		ds.m_format = Ms::DEPTH_RT_PIXEL_FORMAT;
	}

	//
	// Color state
	//
	ColorStateInfo& color = pinit.m_color;
	if(key.m_pass == Pass::SM)
	{
		// Default
	}
	else if(m_mtl->getForwardShading())
	{
		color.m_attachmentCount = 1;
		color.m_attachments[0].m_format = Is::RT_PIXEL_FORMAT;
		color.m_attachments[0].m_srcBlendMethod = BlendMethod::SRC_ALPHA;
		color.m_attachments[0].m_dstBlendMethod =
			BlendMethod::ONE_MINUS_SRC_ALPHA;
	}
	else
	{
		color.m_attachmentCount = Ms::ATTACHMENT_COUNT;
		ANKI_ASSERT(Ms::ATTACHMENT_COUNT == 3);
		color.m_attachments[0].m_format = Ms::RT_PIXEL_FORMATS[0];
		color.m_attachments[1].m_format = Ms::RT_PIXEL_FORMATS[1];
		color.m_attachments[2].m_format = Ms::RT_PIXEL_FORMATS[2];
	}
}

//==============================================================================
// Model                                                                       =
//==============================================================================

//==============================================================================
Model::Model(ResourceManager* manager)
	: ResourceObject(manager)
{
}

//==============================================================================
Model::~Model()
{
	auto alloc = getAllocator();

	for(ModelPatch* patch : m_modelPatches)
	{
		alloc.deleteInstance(patch);
	}

	m_modelPatches.destroy(alloc);
}

//==============================================================================
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
		ANKI_CHECK(
			modelPatchEl.getNextSiblingElement("modelPatch", modelPatchEl));
	} while(modelPatchEl);

	// Check number of model patches
	if(count < 1)
	{
		ANKI_LOGE("Zero number of model patches");
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

		ANKI_CHECK(
			mpatch->create(SArray<CString>(&meshesFnames[0], meshesCount),
				cstr,
				&getManager()));

		m_modelPatches[count++] = mpatch;

		// Move to next
		ANKI_CHECK(
			modelPatchEl.getNextSiblingElement("modelPatch", modelPatchEl));
	} while(modelPatchEl);

	// Calculate compound bounding volume
	RenderingKey key;
	key.m_lod = 0;
	m_visibilityShape = m_modelPatches[0]->getMesh(key).getBoundingShape();

	for(auto it = m_modelPatches.begin() + 1; it != m_modelPatches.end(); ++it)
	{
		m_visibilityShape = m_visibilityShape.getCompoundShape(
			(*it)->getMesh(key).getBoundingShape());
	}

	return ErrorCode::NONE;
}

} // end namespace anki
