// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Gr.h>
#include <anki/collision/Aabb.h>
#include <anki/resource/RenderingKey.h>
#include <anki/resource/MeshResource.h>
#include <anki/resource/MaterialResource.h>

namespace anki
{

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
	VertexAttributeLocation m_location;
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

	Array<ModelVertexBufferBinding, MAX_VERTEX_ATTRIBUTES> m_vertexBufferBindings;
	U32 m_vertexBufferBindingCount;
	Array<ModelVertexAttribute, MAX_VERTEX_ATTRIBUTES> m_vertexAttributes;
	U32 m_vertexAttributeCount;

	BufferPtr m_indexBuffer;
	PtrSize m_indexBufferOffset;
	IndexType m_indexType;
	U32 m_indexCount;

	U32 m_boneTransformsBinding;
	U32 m_prevFrameBoneTransformsBinding;
};

/// Part of the information required to create a TLAS and a SBT.
/// @memberof ModelResource
class ModelRayTracingInfo
{
public:
	ModelGpuDescriptor m_descriptor;
	AccelerationStructurePtr m_bottomLevelAccelerationStructure;
	Array<U32, U(RayType::COUNT)> m_shaderGroupHandleIndices;

	/// Get some pointers that the m_descriptor is pointing to. Use these pointers for life tracking.
	Array<GrObjectPtr, TEXTURE_CHANNEL_COUNT + 2> m_grObjectReferences;
	U32 m_grObjectReferenceCount;
};

/// Model patch interface class. Its very important class and it binds the material with the mesh
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
	void getRayTracingInfo(U32 lod, ModelRayTracingInfo& info) const;

	RayTypeBit getSupportedRayTracingTypes() const
	{
		return m_mtl->getSupportedRayTracingTypes();
	}

private:
	ModelResource* m_model ANKI_DEBUG_CODE(= nullptr);

	MaterialResourcePtr m_mtl;

	Array<MeshResourcePtr, MAX_LOD_COUNT> m_meshes; ///< One for each LOD
	U8 m_meshLodCount = 0;

	ANKI_USE_RESULT Error init(ModelResource* model, ConstWeakArray<CString> meshFNames, const CString& mtlFName,
							   Bool async, ResourceManager* resources);

	ANKI_USE_RESULT Bool supportsSkinning() const
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
/// Requirements:
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

	Bool supportsSkinning() const
	{
		return m_skinning;
	}

	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

private:
	DynamicArray<ModelPatch> m_modelPatches;
	Aabb m_boundingVolume;
	Bool m_skinning = false;
};
/// @}

} // end namespace anki
