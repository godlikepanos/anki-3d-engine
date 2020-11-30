// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/String.h>
#include <anki/util/File.h>
#include <anki/util/HashMap.h>
#include <anki/resource/Common.h>
#include <anki/Math.h>
#include <cgltf/cgltf.h>

namespace anki
{

/// @addtogroup importer
/// @{

#define ANKI_GLTF_LOGI(...) ANKI_LOG("GLTF", NORMAL, __VA_ARGS__)
#define ANKI_GLTF_LOGE(...) ANKI_LOG("GLTF", ERROR, __VA_ARGS__)
#define ANKI_GLTF_LOGW(...) ANKI_LOG("GLTF", WARNING, __VA_ARGS__)
#define ANKI_GLTF_LOGF(...) ANKI_LOG("GLTF", FATAL, __VA_ARGS__)

class GltfImporterInitInfo
{
public:
	CString m_inputFilename;
	CString m_outDirectory;
	CString m_rpath;
	CString m_texrpath;
	Bool m_optimizeMeshes = true;
	F32 m_lodFactor = 1.0f;
	U32 m_lodCount = 1;
	F32 m_lightIntensityScale = 1.0f;
	U32 m_threadCount = MAX_U32;
	CString m_comment;
};

/// Import GLTF and spit AnKi scenes.
class GltfImporter
{
public:
	GltfImporter(GenericMemoryPoolAllocator<U8> alloc);

	~GltfImporter();

	ANKI_USE_RESULT Error init(const GltfImporterInitInfo& initInfo);

	ANKI_USE_RESULT Error writeAll();

private:
	class PtrHasher
	{
	public:
		U64 operator()(const void* ptr)
		{
			return computeHash(&ptr, sizeof(ptr));
		}
	};

	// Data
	static const char* XML_HEADER;

	GenericMemoryPoolAllocator<U8> m_alloc;

	StringAuto m_inputFname = {m_alloc};
	StringAuto m_outDir = {m_alloc};
	StringAuto m_rpath = {m_alloc};
	StringAuto m_texrpath = {m_alloc};

	cgltf_data* m_gltf = nullptr;

	F32 m_normalsMergeAngle = toRad(30.0f);

	ThreadHive* m_hive = nullptr;

	File m_sceneFile;

	Atomic<I32> m_errorInThread{0};

	HashMapAuto<const void*, U32, PtrHasher> m_nodePtrToIdx{m_alloc}; ///< Need an index for the unnamed nodes.

	F32 m_lodFactor = 1.0f;
	U32 m_lodCount = 1;
	F32 m_lightIntensityScale = 1.0f;
	Bool m_optimizeMeshes = false;
	StringAuto m_comment{m_alloc};

	/// Don't generate LODs for meshes with less vertices than this number.
	U32 m_skipLodVertexCountThreshold = 256;

	// Misc
	ANKI_USE_RESULT Error getExtras(const cgltf_extras& extras, HashMapAuto<CString, StringAuto>& out);
	ANKI_USE_RESULT Error parseArrayOfNumbers(CString str, DynamicArrayAuto<F64>& out,
											  const U32* expectedArraySize = nullptr);
	void populateNodePtrToIdx();
	void populateNodePtrToIdxInternal(const cgltf_node& node, U32& idx);
	StringAuto getNodeName(const cgltf_node& node);

	template<typename T, typename TFunc>
	static void visitAccessor(const cgltf_accessor& accessor, TFunc func);

	template<typename T>
	static void readAccessor(const cgltf_accessor& accessor, DynamicArrayAuto<T>& out)
	{
		visitAccessor<T>(accessor, [&](const T& val) { out.emplaceBack(val); });
	}

	StringAuto fixFilename(CString in) const
	{
		StringAuto out(m_alloc, in);
		out.replaceAll("|", "_");
		out.replaceAll(" ", "_");
		return out;
	}

	F32 computeLodFactor(U32 lod) const
	{
		ANKI_ASSERT(lod < m_lodCount);
		return 1.0f - m_lodFactor * F32(lod);
	}

	Bool skipMeshLod(const cgltf_mesh& mesh, U32 lod) const
	{
		return U32(computeLodFactor(lod) * F32(getMeshTotalVertexCount(mesh))) < m_skipLodVertexCountThreshold;
	}

	static U32 getMeshTotalVertexCount(const cgltf_mesh& mesh);

	// Resources
	ANKI_USE_RESULT Error writeMesh(const cgltf_mesh& mesh, CString nameOverride, F32 decimateFactor);
	ANKI_USE_RESULT Error writeMaterial(const cgltf_material& mtl, RayTypeBit usedRayTypes);
	ANKI_USE_RESULT Error writeModel(const cgltf_mesh& mesh, CString skinName);
	ANKI_USE_RESULT Error writeAnimation(const cgltf_animation& anim);
	ANKI_USE_RESULT Error writeSkeleton(const cgltf_skin& skin);
	ANKI_USE_RESULT Error writeCollisionMesh(const cgltf_mesh& mesh, U32 maxLod);

	// Scene
	ANKI_USE_RESULT Error writeTransform(const Transform& trf);
	ANKI_USE_RESULT Error visitNode(const cgltf_node& node, const Transform& parentTrf,
									const HashMapAuto<CString, StringAuto>& parentExtras);
	ANKI_USE_RESULT Error writeLight(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras);
	ANKI_USE_RESULT Error writeCamera(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras);
	ANKI_USE_RESULT Error writeModelNode(const cgltf_node& node, const HashMapAuto<CString, StringAuto>& parentExtras);
};
/// @}

template<typename T, typename TFunc>
void GltfImporter::visitAccessor(const cgltf_accessor& accessor, TFunc func)
{
	const U8* base =
		static_cast<const U8*>(accessor.buffer_view->buffer->data) + accessor.offset + accessor.buffer_view->offset;

	PtrSize stride = accessor.buffer_view->stride;
	if(stride == 0)
	{
		stride = accessor.stride;
	}
	ANKI_ASSERT(stride);
	ANKI_ASSERT(stride >= sizeof(T));

	const U count = accessor.count;

	for(U i = 0; i < count; ++i)
	{
		const U8* ptr = base + stride * i;
		T val;
		memcpy(&val, ptr, sizeof(T)); // Memcpy because the input buffer might not be aligned
		func(val);
	}
}

} // end namespace anki
