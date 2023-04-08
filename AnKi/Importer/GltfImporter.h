// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
	Bool m_importTextures = false;
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
	// Data
	mutable BaseMemoryPool* m_pool = nullptr;

	ImporterString m_inputFname = {m_pool};
	ImporterString m_outDir = {m_pool};
	ImporterString m_rpath = {m_pool};
	ImporterString m_texrpath = {m_pool};

	cgltf_data* m_gltf = nullptr;

	F32 m_normalsMergeAngle = toRad(30.0f);

	ThreadHive* m_hive = nullptr;

	File m_sceneFile;

	mutable Atomic<I32> m_errorInThread = {0};

	ImporterHashMap<const void*, U32> m_nodePtrToIdx = {m_pool}; ///< Need an index for the unnamed nodes.

	F32 m_lodFactor = 1.0f;
	U32 m_lodCount = 1;
	F32 m_lightIntensityScale = 1.0f;
	Bool m_optimizeMeshes = false;
	Bool m_optimizeAnimations = false;
	ImporterString m_comment = {m_pool};

	/// Don't generate LODs for meshes with less vertices than this number.
	U32 m_skipLodVertexCountThreshold = 256;

	Bool m_importTextures = false;

	template<typename T>
	class ImportRequest
	{
	public:
		const GltfImporter* m_importer = nullptr;
		T m_value = {};
	};

	class MaterialImportRequest
	{
	public:
		const cgltf_material* m_cgltfMaterial = nullptr;
		Bool m_writeRt = true;

		Bool operator==(const MaterialImportRequest& b) const
		{
			return m_cgltfMaterial == b.m_cgltfMaterial && m_writeRt == b.m_writeRt;
		}
	};

	ImporterDynamicArray<ImportRequest<const cgltf_mesh*>> m_meshImportRequests = {m_pool};
	ImporterDynamicArray<ImportRequest<MaterialImportRequest>> m_materialImportRequests = {m_pool};
	ImporterDynamicArray<ImportRequest<const cgltf_skin*>> m_skinImportRequests = {m_pool};
	ImporterDynamicArray<ImportRequest<const cgltf_mesh*>> m_modelImportRequests = {m_pool};

	// Misc
	template<typename T>
	void addRequest(const T& value, ImporterDynamicArray<ImportRequest<T>>& array) const
	{
		Bool found = false;
		for(const auto& req : array)
		{
			ANKI_ASSERT(req.m_importer == this);
			if(req.m_value == value)
			{
				found = true;
				break;
			}
		}

		if(!found)
		{
			const ImportRequest<T> req = {this, value};
			array.emplaceBack(req);
		}
	}

	Error getExtras(const cgltf_extras& extras, ImporterHashMap<CString, ImporterString>& out) const;
	Error parseArrayOfNumbers(CString str, ImporterDynamicArray<F64>& out, const U32* expectedArraySize = nullptr);
	void populateNodePtrToIdx();
	void populateNodePtrToIdxInternal(const cgltf_node& node, U32& idx);
	ImporterString getNodeName(const cgltf_node& node) const;

	template<typename T, typename TFunc>
	static void visitAccessor(const cgltf_accessor& accessor, TFunc func);

	template<typename T>
	static void readAccessor(const cgltf_accessor& accessor, ImporterDynamicArray<T>& out)
	{
		visitAccessor<T>(accessor, [&](const T& val) {
			out.emplaceBack(val);
		});
	}

	ImporterString fixFilename(CString in) const
	{
		ImporterString out(in, m_pool);
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
	ImporterString computeModelResourceFilename(const cgltf_mesh& mesh) const;
	ImporterString computeMeshResourceFilename(const cgltf_mesh& mesh) const;
	ImporterString computeMaterialResourceFilename(const cgltf_material& mtl) const;
	ImporterString computeAnimationResourceFilename(const cgltf_animation& anim) const;
	ImporterString computeSkeletonResourceFilename(const cgltf_skin& skin) const;

	// Resources
	Error writeMesh(const cgltf_mesh& mesh) const;
	Error writeMaterial(const cgltf_material& mtl, Bool writeRayTracing) const;
	Error writeModel(const cgltf_mesh& mesh) const;
	Error writeAnimation(const cgltf_animation& anim);
	Error writeSkeleton(const cgltf_skin& skin) const;

	// Scene
	Error writeTransform(const Transform& trf);
	Error visitNode(const cgltf_node& node, const Transform& parentTrf,
					const ImporterHashMap<CString, ImporterString>& parentExtras);
	Error writeLight(const cgltf_node& node, const ImporterHashMap<CString, ImporterString>& parentExtras);
	Error writeCamera(const cgltf_node& node, const ImporterHashMap<CString, ImporterString>& parentExtras);
	Error writeModelNode(const cgltf_node& node, const ImporterHashMap<CString, ImporterString>& parentExtras);
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
