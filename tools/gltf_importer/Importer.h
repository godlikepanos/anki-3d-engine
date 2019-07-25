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

	F32 m_normalsMergeAngle = toRad(30.0f);

	ThreadHive* m_hive = nullptr;

	File m_sceneFile;

	Atomic<I32> m_errorInThread{0};

	class PtrHasher
	{
	public:
		U64 operator()(const void* ptr)
		{
			return computeHash(&ptr, sizeof(ptr));
		}
	};

	HashMapAuto<const void*, U32, PtrHasher> m_nodePtrToIdx{m_alloc}; ///< Need an index for the unnamed nodes.

	ANKI_USE_RESULT Error getExtras(const cgltf_extras& extras, HashMapAuto<CString, StringAuto>& out);
	ANKI_USE_RESULT Error parseArrayOfNumbers(
		CString str, DynamicArrayAuto<F64>& out, const U* expectedArraySize = nullptr);
	void populateNodePtrToIdx();
	void populateNodePtrToIdx(const cgltf_node& node, U& idx);
	StringAuto getNodeName(const cgltf_node& node);

	ANKI_USE_RESULT Error writeMesh(const cgltf_mesh& mesh);
	ANKI_USE_RESULT Error writeMaterial(const cgltf_material& mtl);
	ANKI_USE_RESULT Error writeModel(const cgltf_mesh& mesh);
	ANKI_USE_RESULT Error writeAnimation(const cgltf_animation& anim);

	// Scene
	ANKI_USE_RESULT Error writeTransform(const Transform& trf);
	ANKI_USE_RESULT Error visitNode(
		const cgltf_node& node, const Transform& parentTrf, const HashMapAuto<CString, StringAuto>& parentExtras);
	ANKI_USE_RESULT Error writeLight(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras);
	ANKI_USE_RESULT Error writeCamera(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras);
	ANKI_USE_RESULT Error writeModelNode(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras);
};

#define ANKI_GLTF_LOGI(...) ANKI_LOG("GLTF", NORMAL, __VA_ARGS__)
#define ANKI_GLTF_LOGE(...) ANKI_LOG("GLTF", ERROR, __VA_ARGS__)
#define ANKI_GLTF_LOGW(...) ANKI_LOG("GLTF", WARNING, __VA_ARGS__)
#define ANKI_GLTF_LOGF(...) ANKI_LOG("GLTF", FATAL, __VA_ARGS__)

} // end namespace anki