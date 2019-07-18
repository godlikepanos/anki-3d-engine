// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Importer.h"

namespace anki
{

Importer::Importer()
{
	m_hive = m_alloc.newInstance<ThreadHive>(getCpuCoresCount(), m_alloc, true);
}

Importer::~Importer()
{
	if(m_gltf)
	{
		cgltf_free(m_gltf);
		m_gltf = nullptr;
	}

	m_alloc.deleteInstance(m_hive);
}

Error Importer::load(CString inputFname, CString outDir, CString rpath, CString texrpath)
{
	m_inputFname.create(inputFname);
	m_outDir.create(outDir);
	m_rpath.create(rpath);
	m_texrpath.create(texrpath);

	cgltf_options options = {};
	cgltf_result res = cgltf_parse_file(&options, inputFname.cstr(), &m_gltf);
	if(res != cgltf_result_success)
	{
		ANKI_LOGE("Failed to open the GLTF file. Code: %d", res);
		return Error::FUNCTION_FAILED;
	}

	res = cgltf_load_buffers(&options, m_gltf, inputFname.cstr());
	if(res != cgltf_result_success)
	{
		ANKI_LOGE("Failed to load GLTF data. Code: %d", res);
		return Error::FUNCTION_FAILED;
	}

	return Error::NONE;
}

Error Importer::writeAll()
{
	// Export meshes
	for(U i = 0; i < m_gltf->meshes_count; ++i)
	{
		ANKI_CHECK(writeMesh(m_gltf->meshes[i]));
	}

	// Export materials
	for(U i = 0; i < m_gltf->materials_count; ++i)
	{
		ANKI_CHECK(writeMaterial(m_gltf->materials[i]));
	}

	StringAuto sceneFname(m_alloc);
	sceneFname.sprintf("%sscene.lua", m_outDir.cstr());
	ANKI_CHECK(m_sceneFile.open(sceneFname.toCString(), FileOpenFlag::WRITE));

	// Nodes
	for(const cgltf_scene* scene = m_gltf->scenes; scene < m_gltf->scenes + m_gltf->scenes_count; ++scene)
	{
		for(cgltf_node* const* node = scene->nodes; node < scene->nodes + scene->nodes_count; ++node)
		{
			ANKI_CHECK(visitNode(*(*node)));
		}
	}

	return Error::NONE;
}

Error Importer::visitNode(const cgltf_node& node)
{
	if(node.light)
	{
		ANKI_CHECK(writeLight(node));
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	return Error::NONE;
}

Error Importer::writeLight(const cgltf_node& node)
{
	const cgltf_light& light = *node.light;
	ANKI_GLTF_LOGI("Exporting light %s", light.name);

	CString lightTypeStr;
	switch(light.type)
	{
	case cgltf_light_type_point:
		lightTypeStr = "Point";
		break;
	case cgltf_light_type_spot:
		lightTypeStr = "Spot";
		break;
	case cgltf_light_type_directional:
		lightTypeStr = "Directional";
		break;
	default:
		ANKI_GLTF_LOGE("Unsupporter light type %d", light.type);
		return Error::USER_DATA;
	}

	ANKI_CHECK(m_sceneFile.writeText("\nnode = scene:new%sLightNode(\"%s\")\n", lightTypeStr.cstr(), light.name));
	ANKI_CHECK(m_sceneFile.writeText("lcomp = node:getSceneNodeBase():getLightComponent()\n"));

	return Error::NONE;
}

} // end namespace anki