// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Resource/RenderingKey.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/MaterialResource.h>

namespace anki {

/// @addtogroup resource
/// @{

/// @memberof ModelResource
class ModelVertexBufferBinding
{
public:
	BufferPtr m_buffer;
	PtrSize m_offset;
	PtrSize m_stride;

	Bool operator==(const ModelVertexBufferBinding& b) const
	{
		return m_buffer == b.m_buffer && m_offset == b.m_offset && m_stride == b.m_stride;
	}

	Bool operator!=(const ModelVertexBufferBinding& b) const
	{
		return !(*this == b);
	}
};

/// @memberof ModelResource
class ModelVertexAttribute
{
public:
	VertexAttributeId m_location;
	Format m_format;
	U32 m_bufferBinding;
	U32 m_relativeOffset;

	Bool operator==(const ModelVertexAttribute& b) const
	{
		return m_bufferBinding == b.m_bufferBinding && m_format == b.m_format && m_relativeOffset == b.m_relativeOffset
			   && m_location == b.m_location;
	}

	Bool operator!=(const ModelVertexAttribute& b) const
	{
		return !(*this == b);
	}
};

/// @memberof ModelResource
/// Part of the information required render the model.
class ModelRenderingInfo
{
public:
	ShaderProgramPtr m_program;

	Array<ModelVertexBufferBinding, kMaxVertexAttributes> m_vertexBufferBindings;
	U32 m_vertexBufferBindingCount;
	Array<ModelVertexAttribute, kMaxVertexAttributes> m_vertexAttributes;
	U32 m_vertexAttributeCount;

	BufferPtr m_indexBuffer;
	PtrSize m_indexBufferOffset;
	IndexType m_indexType;
	U32 m_firstIndex;
	U32 m_indexCount;
};

/// Part of the information required to create a TLAS and a SBT.
/// @memberof ModelResource
class ModelRayTracingInfo
{
public:
	AccelerationStructurePtr m_bottomLevelAccelerationStructure;
	U32 m_shaderGroupHandleIndex;

	/// Get some pointers to pass to the command buffer for refcounting.
	ConstWeakArray<GrObjectPtr> m_grObjectReferences;
};

/// Model patch class. Its very important class and it binds a material with a few mesh (one for each LOD).
class ModelPatch
{
	friend class ModelResource;

public:
	const MaterialResourcePtr& getMaterial() const
	{
		return m_mtl;
	}

	const MeshResourcePtr& getMesh(U32 lod) const
	{
		return m_meshes[lod];
	}

	const Aabb& getBoundingShape() const
	{
		return m_meshes[0]->getBoundingShape();
	}

	/// Get information for rendering.
	void getRenderingInfo(const RenderingKey& key, ModelRenderingInfo& inf) const;

	/// Get the ray tracing info.
	void getRayTracingInfo(const RenderingKey& key, ModelRayTracingInfo& info) const;

private:
#if ANKI_ENABLE_ASSERTIONS
	ModelResource* m_model = nullptr;
#endif
	MaterialResourcePtr m_mtl;
	Array<MeshResourcePtr, kMaxLodCount> m_meshes; ///< Just keep the references.
	DynamicArray<GrObjectPtr> m_grObjectRefs;

	// Begin cached data
	class VertexAttributeInfo
	{
	public:
		U32 m_bufferBinding : 8;
		U32 m_relativeOffset : 24;
		Format m_format = Format::kNone;
	};

	Array<VertexAttributeInfo, U(VertexAttributeId::kCount)> m_vertexAttributeInfos;

	class VertexBufferInfo
	{
	public:
		BufferPtr m_buffer;
		PtrSize m_stride : 16;
		PtrSize m_offset : 48;
	};

	Array2d<VertexBufferInfo, kMaxLodCount, U(VertexAttributeBufferId::kCount)> m_vertexBufferInfos;

	class IndexBufferInfo
	{
	public:
		BufferPtr m_buffer;
		PtrSize m_offset;
		U32 m_firstIndex = kMaxU32;
		U32 m_indexCount = kMaxU32;
	};

	Array<IndexBufferInfo, kMaxLodCount> m_indexBufferInfos;
	BitSet<U(VertexAttributeId::kCount)> m_presentVertexAttributes = {false};
	IndexType m_indexType : 2;
	// End cached data

	U8 m_meshLodCount : 6;

	Error init(ModelResource* model, ConstWeakArray<CString> meshFNames, const CString& mtlFName, U32 subMeshIndex,
			   Bool async, ResourceManager* resources);

	[[nodiscard]] Bool supportsSkinning() const
	{
		return m_meshes[0]->hasBoneWeights() && m_mtl->supportsSkinning();
	}
};

/// Model is an entity that acts as a container for other resources. Models are all the non static objects in a map.
///
/// XML file format:
/// @code
/// <model>
/// 	<modelPatches>
/// 		<modelPatch [subMeshIndex=int]>
/// 			<mesh>path/to/mesh.mesh</mesh>
///				[<mesh1>path/to/mesh_lod_1.ankimesh</mesh1>]
///				[<mesh2>path/to/mesh_lod_2.ankimesh</mesh2>]
/// 			<material>path/to/material.ankimtl</material>
/// 		</modelPatch>
/// 		...
/// 		<modelPatch>...</modelPatch>
/// 	</modelPatches>
/// </model>
/// @endcode
///
/// Notes:
/// - If the materials need texture coords then mesh should have them
/// - If the subMeshIndex is not present then assume the whole mesh
class ModelResource : public ResourceObject
{
public:
	ModelResource(ResourceManager* manager);

	~ModelResource();

	ConstWeakArray<ModelPatch> getModelPatches() const
	{
		return m_modelPatches;
	}

	/// The volume that includes all the geometry of all model patches.
	const Aabb& getBoundingVolume() const
	{
		return m_boundingVolume;
	}

	Error load(const ResourceFilename& filename, Bool async);

private:
	DynamicArray<ModelPatch> m_modelPatches;
	Aabb m_boundingVolume;
};
/// @}

} // end namespace anki
