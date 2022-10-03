// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Importer/Common.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/HashMap.h>
#include <AnKi/Resource/Common.h>
#include <AnKi/Math.h>
#include <Cgltf/cgltf.h>

namespace anki {

/// @addtogroup importer
/// @{

/// @memberof GltfImporter
class GltfImporterInitInfo
{
public:
	CString m_inputFilename;
	CString m_outDirectory;
	CString m_rpath;
	CString m_texrpath;
	Bool m_optimizeMeshes = true;
	Bool m_optimizeAnimations = true;
	F32 m_lodFactor = 1.0f;
	U32 m_lodCount = 1;
	F32 m_lightIntensityScale = 1.0f;
	U32 m_threadCount = kMaxU32;
	CString m_comment;
};

/// Import GLTF and spit AnKi scenes.
class GltfImporter
{
public:
	GltfImporter(BaseMemoryPool* pool);

	~GltfImporter();

	Error init(const GltfImporterInitInfo& initInfo);

	Error writeAll();

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
	BaseMemoryPool* m_pool = nullptr;

	StringRaii m_inputFname = {m_pool};
	StringRaii m_outDir = {m_pool};
	StringRaii m_rpath = {m_pool};
	StringRaii m_texrpath = {m_pool};

	cgltf_data* m_gltf = nullptr;

	F32 m_normalsMergeAngle = toRad(30.0f);

	ThreadHive* m_hive = nullptr;

	File m_sceneFile;

	Atomic<I32> m_errorInThread{0};

	HashMapRaii<const void*, U32, PtrHasher> m_nodePtrToIdx = {m_pool}; ///< Need an index for the unnamed nodes.

	F32 m_lodFactor = 1.0f;
	U32 m_lodCount = 1;
	F32 m_lightIntensityScale = 1.0f;
	Bool m_optimizeMeshes = false;
	Bool m_optimizeAnimations = false;
	StringRaii m_comment = {m_pool};

	/// Don't generate LODs for meshes with less vertices than this number.
	U32 m_skipLodVertexCountThreshold = 256;

	// Misc
	Error getExtras(const cgltf_extras& extras, HashMapRaii<CString, StringRaii>& out);
	Error parseArrayOfNumbers(CString str, DynamicArrayRaii<F64>& out, const U32* expectedArraySize = nullptr);
	void populateNodePtrToIdx();
	void populateNodePtrToIdxInternal(const cgltf_node& node, U32& idx);
	StringRaii getNodeName(const cgltf_node& node);

	template<typename T, typename TFunc>
	static void visitAccessor(const cgltf_accessor& accessor, TFunc func);

	template<typename T>
	static void readAccessor(const cgltf_accessor& accessor, DynamicArrayRaii<T>& out)
	{
		visitAccessor<T>(accessor, [&](const T& val) {
			out.emplaceBack(val);
		});
	}

	StringRaii fixFilename(CString in) const
	{
		StringRaii out(m_pool, in);
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

	// Compute filenames for various resources. Use a hash to solve the casing issue and remove unwanted special chars
	StringRaii computeModelResourceFilename(const cgltf_mesh& mesh) const;
	StringRaii computeMeshResourceFilename(const cgltf_mesh& mesh, U32 lod = 0) const;
	StringRaii computeMaterialResourceFilename(const cgltf_material& mtl) const;
	StringRaii computeAnimationResourceFilename(const cgltf_animation& anim) const;
	StringRaii computeSkeletonResourceFilename(const cgltf_skin& skin) const;

	// Resources
	Error writeMesh(const cgltf_mesh& mesh, U32 lod, F32 decimateFactor);
	Error writeMaterial(const cgltf_material& mtl, Bool writeRayTracing);
	Error writeModel(const cgltf_mesh& mesh);
	Error writeAnimation(const cgltf_animation& anim);
	Error writeSkeleton(const cgltf_skin& skin);

	// Scene
	Error writeTransform(const Transform& trf);
	Error visitNode(const cgltf_node& node, const Transform& parentTrf,
					const HashMapRaii<CString, StringRaii>& parentExtras);
	Error writeLight(const cgltf_node& node, const HashMapRaii<CString, StringRaii>& parentExtras);
	Error writeCamera(const cgltf_node& node, const HashMapRaii<CString, StringRaii>& parentExtras);
	Error writeModelNode(const cgltf_node& node, const HashMapRaii<CString, StringRaii>& parentExtras);
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

	const U count = U(accessor.count);

	for(U i = 0; i < count; ++i)
	{
		const U8* ptr = base + stride * i;
		T val;
		memcpy(&val, ptr, sizeof(T)); // Memcpy because the input buffer might not be aligned
		func(val);
	}
}

} // end namespace anki
