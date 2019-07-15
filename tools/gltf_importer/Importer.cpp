// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Importer.h"

namespace anki
{

Importer::~Importer()
{
	if(m_gltf)
	{
		cgltf_free(m_gltf);
		m_gltf = nullptr;
	}
}

Error Importer::load(CString inputFname, CString outDir)
{
	m_inputFname.create(inputFname);
	m_outDir.create(outDir);

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

	return Error::NONE;
}

} // end namespace anki