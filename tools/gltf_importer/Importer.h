// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <src/anki/AnKi.h>
#include <cgltf/cgltf.h>

namespace anki
{

/// Import GLTF and spit AnKi scenes.
class Importer
{
public:
	Importer();

	~Importer();

	Error load(CString inputFname, CString outDir);

	Error writeAll();

private:
	HeapAllocator<U8> m_alloc{allocAligned, nullptr};

	StringAuto m_inputFname = {m_alloc};
	StringAuto m_outDir = {m_alloc};

	cgltf_data* m_gltf = nullptr;

	F32 m_normalsMergeCosAngle = cos(toRad(30.0f));

	ThreadHive* m_hive = nullptr;

	ANKI_USE_RESULT Error writeMesh(const cgltf_mesh& mesh);
	ANKI_USE_RESULT Error writeMaterial(const cgltf_material& mtl);
};

#define ANKI_GLTF_LOGI(...) ANKI_LOG("GLTF", NORMAL, __VA_ARGS__)
#define ANKI_GLTF_LOGE(...) ANKI_LOG("GLTF", ERROR, __VA_ARGS__)
#define ANKI_GLTF_LOGW(...) ANKI_LOG("GLTF", WARNING, __VA_ARGS__)
#define ANKI_GLTF_LOGF(...) ANKI_LOG("GLTF", FATAL, __VA_ARGS__)

} // end namespace anki