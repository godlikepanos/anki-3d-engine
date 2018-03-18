// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Gr.h>
#include <anki/collision/Obb.h>
#include <anki/resource/RenderingKey.h>
#include <anki/resource/MeshResource.h>
#include <anki/resource/MaterialResource.h>
#include <anki/resource/SkeletonResource.h>
#include <anki/resource/AnimationResource.h>

namespace anki
{

// Forward
class PhysicsCollisionShape;

/// @addtogroup resource
/// @{

class VertexBufferBinding
{
public:
	BufferPtr m_buffer;
	PtrSize m_offset;
	PtrSize m_stride;

	Bool operator==(const VertexBufferBinding& b) const
	{
		return m_buffer == b.m_buffer && m_offset == b.m_offset && m_stride == b.m_stride;
	}

	Bool operator!=(const VertexBufferBinding& b) const
	{
		return !(*this == b);
	}
};

class VertexAttributeInfo
{
public:
	U32 m_bufferBinding;
	VertexAttributeLocation m_location;
	Format m_format;
	PtrSize m_relativeOffset;

	Bool operator==(const VertexAttributeInfo& b) const
	{
		return m_bufferBinding == b.m_bufferBinding && m_format == b.m_format && m_relativeOffset == b.m_relativeOffset
			   && m_location == b.m_location;
	}

	Bool operator!=(const VertexAttributeInfo& b) const
	{
		return !(*this == b);
	}
};

class ModelRenderingInfo
{
public:
	Array<U32, MAX_SUB_DRAWCALLS> m_indicesCountArray;
	Array<PtrSize, MAX_SUB_DRAWCALLS> m_indicesOffsetArray;
	U32 m_drawcallCount;

	ShaderProgramPtr m_program;

	Array<VertexBufferBinding, MAX_VERTEX_ATTRIBUTES> m_vertexBufferBindings;
	U32 m_vertexBufferBindingCount;
	Array<VertexAttributeInfo, MAX_VERTEX_ATTRIBUTES> m_vertexAttributes;
	U32 m_vertexAttributeCount;

	BufferPtr m_indexBuffer;
	PtrSize m_indexBufferOffset;
	IndexType m_indexType;
};

/// Model patch interface class. Its very important class and it binds the material with the mesh
class ModelPatch
{
public:
	ModelPatch(ModelResource* model);

	~ModelPatch();

	MaterialResourcePtr getMaterial() const
	{
		return m_mtl;
	}

	const MeshResource& getMesh(const RenderingKey& key) const
	{
		return *m_meshes[key.m_lod];
	}

	const ModelResource& getModel() const
	{
		ANKI_ASSERT(m_model);
		return *m_model;
	}

	const Obb& getBoundingShape() const
	{
		return m_meshes[0]->getBoundingShape();
	}

	const Obb& getBoundingShapeSub(U32 subMeshId) const
	{
		U32 firstIdx, idxCount;
		const Obb* obb;
		m_meshes[0]->getSubMeshInfo(subMeshId, firstIdx, idxCount, obb);
		return *obb;
	}

	U32 getSubMeshCount() const
	{
		return m_meshes[0]->getSubMeshCount();
	}

	ANKI_USE_RESULT Error create(
		ConstWeakArray<CString> meshFNames, const CString& mtlFName, Bool async, ResourceManager* resources);

	/// Get information for multiDraw rendering. Given an array of submeshes that are visible return the correct indices
	/// offsets and counts.
	void getRenderingDataSub(const RenderingKey& key, WeakArray<U8> subMeshIndicesArray, ModelRenderingInfo& inf) const;

private:
	ModelResource* m_model ANKI_DBG_NULLIFY;

	Array<MeshResourcePtr, MAX_LOD_COUNT> m_meshes; ///< One for each LOD
	U8 m_meshCount = 0;
	MaterialResourcePtr m_mtl;

	/// Return the maximum number of LODs
	U getLodCount() const;
};

/// Model is an entity that acts as a container for other resources. Models are all the non static objects in a map.
///
/// XML file format:
/// @code
/// <model>
/// 	<modelPatches>
/// 		<modelPatch>
/// 			<mesh>path/to/mesh.mesh</mesh>
///				[<mesh1>path/to/mesh_lod_1.mesh</mesh1>]
///				[<mesh2>path/to/mesh_lod_2.mesh</mesh2>]
/// 			<material>path/to/material.mtl</material>
/// 		</modelPatch>
/// 		...
/// 		<modelPatch>...</modelPatch>
/// 	</modelPatches>
/// 	[<skeleton>path/to/skeleton.skel</skeleton>]
/// 	[<skeletonAnimations>
/// 		<animation>path/to/animation.anim</animation>
/// 		...
/// 	</skeletonAnimations>]
/// </model>
/// @endcode
///
/// Requirements:
/// - If the materials need texture coords then mesh should have them
/// - The skeleton and skelAnims are optional
/// - Its an error to have skelAnims without skeleton
class ModelResource : public ResourceObject
{
public:
	ModelResource(ResourceManager* manager);

	~ModelResource();

	const DynamicArray<ModelPatch*>& getModelPatches() const
	{
		return m_modelPatches;
	}

	const Obb& getVisibilityShape() const
	{
		return m_visibilityShape;
	}

	SkeletonResourcePtr getSkeleton() const
	{
		return m_skeleton;
	}

	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

private:
	DynamicArray<ModelPatch*> m_modelPatches;
	Obb m_visibilityShape;
	SkeletonResourcePtr m_skeleton;
	DynamicArray<AnimationResourcePtr> m_animations;
};
/// @}

} // end namespace anki
