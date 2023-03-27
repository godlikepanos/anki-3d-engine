// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
/// Part of the information required render the model.
class ModelRenderingInfo
{
public:
	ShaderProgramPtr m_program;

	PtrSize m_indexBufferOffset;
	IndexType m_indexType;
	U32 m_firstIndex;
	U32 m_indexCount;

	/// Offset to the vertex buffer or kMaxPtrSize if stream is not present.
	Array<PtrSize, U32(VertexStreamId::kMeshRelatedCount)> m_vertexBufferOffsets;
};

/// Part of the information required to create a TLAS and a SBT.
/// @memberof ModelResource
class ModelRayTracingInfo
{
public:
	AccelerationStructurePtr m_bottomLevelAccelerationStructure;
	U32 m_shaderGroupHandleIndex;

	PtrSize m_indexBufferOffset;
};

/// Model patch class. Its very important class and it binds a material with a mesh.
class ModelPatch
{
	friend class ModelResource;

public:
	const MaterialResourcePtr& getMaterial() const
	{
		return m_mtl;
	}

	const MeshResourcePtr& getMesh() const
	{
		return m_mesh;
	}

	const Aabb& getBoundingShape() const
	{
		return m_aabb;
	}

	/// Get information for rendering.
	void getRenderingInfo(const RenderingKey& key, ModelRenderingInfo& inf) const;

	/// Get the ray tracing info.
	void getRayTracingInfo(const RenderingKey& key, ModelRayTracingInfo& info) const;

private:
	class Lod
	{
	public:
		PtrSize m_indexBufferOffset = kMaxPtrSize;
		U32 m_firstIndex = kMaxU32;
		U32 m_indexCount = kMaxU32;

		Array<PtrSize, U32(VertexStreamId::kMeshRelatedCount)> m_vertexBufferOffsets = {};
	};

#if ANKI_ENABLE_ASSERTIONS
	ModelResource* m_model = nullptr;
#endif
	MaterialResourcePtr m_mtl;
	MeshResourcePtr m_mesh; ///< Just keep the references.

	Array<Lod, kMaxLodCount> m_lodInfos;
	Aabb m_aabb;
	U32 m_meshLodCount = 0;

	[[nodiscard]] Bool supportsSkinning() const
	{
		return m_mesh->isVertexStreamPresent(VertexStreamId::kBoneIds) && m_mtl->supportsSkinning();
	}

	Error init(ModelResource* model, CString meshFName, const CString& mtlFName, U32 subMeshIndex, Bool async);
};

/// Model is an entity that acts as a container for other resources. Models are all the non static objects in a map.
///
/// XML file format:
/// @code
/// <model>
/// 	<modelPatches>
/// 		<modelPatch>
/// 			<mesh [subMeshIndex=int]>path/to/mesh.ankimesh</mesh>
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
	ModelResource() = default;

	~ModelResource() = default;

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
	ResourceDynamicArray<ModelPatch> m_modelPatches;
	Aabb m_boundingVolume;
};
/// @}

} // end namespace anki
