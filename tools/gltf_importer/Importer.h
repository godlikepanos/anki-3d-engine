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

	Error load(CString inputFname, CString outDir, CString rpath, CString texrpath);

	Error writeAll();

private:
	HeapAllocator<U8> m_alloc{allocAligned, nullptr};

	StringAuto m_inputFname = {m_alloc};
	StringAuto m_outDir = {m_alloc};
	StringAuto m_rpath = {m_alloc};
	StringAuto m_texrpath = {m_alloc};

	cgltf_data* m_gltf = nullptr;

	F32 m_normalsMergeCosAngle = cos(toRad(30.0f));

	ThreadHive* m_hive = nullptr;

	File m_sceneFile;

	ANKI_USE_RESULT Error getExtras(const cgltf_extras& extras, HashMapAuto<StringAuto, StringAuto>& out);
	ANKI_USE_RESULT Error parseArrayOfNumbers(
		CString str, DynamicArrayAuto<F64>& out, const U* expectedArraySize = nullptr);

	static F32 computeLightRadius(const Vec3 color)
	{
		// Based on the attenuation equation: att = 1 - fragLightDist^2 / lightRadius^2
		const F32 minAtt = 0.01f;
		const F32 maxIntensity = max(max(color.x(), color.y()), color.z());
		return sqrt(maxIntensity / minAtt);
	}

	ANKI_USE_RESULT Error writeMesh(const cgltf_mesh& mesh);
	ANKI_USE_RESULT Error writeMaterial(const cgltf_material& mtl);
	ANKI_USE_RESULT Error writeModel(const cgltf_mesh& mesh);

	ANKI_USE_RESULT Error visitNode(const cgltf_node& node);
	ANKI_USE_RESULT Error writeTransform(const cgltf_node& node);
	ANKI_USE_RESULT Error writeLight(const cgltf_node& node);
	ANKI_USE_RESULT Error writeCamera(const cgltf_node& node);
	ANKI_USE_RESULT Error writeModelNode(const cgltf_node& node);
};

#define ANKI_GLTF_LOGI(...) ANKI_LOG("GLTF", NORMAL, __VA_ARGS__)
#define ANKI_GLTF_LOGE(...) ANKI_LOG("GLTF", ERROR, __VA_ARGS__)
#define ANKI_GLTF_LOGW(...) ANKI_LOG("GLTF", WARNING, __VA_ARGS__)
#define ANKI_GLTF_LOGF(...) ANKI_LOG("GLTF", FATAL, __VA_ARGS__)

} // end namespace anki