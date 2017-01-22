// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>
#include <anki/Gr.h>
#include <anki/collision/Obb.h>
#include <anki/resource/RenderingKey.h>
#include <anki/resource/Mesh.h>
#include <anki/resource/Material.h>
#include <anki/resource/Skeleton.h>
#include <anki/resource/Animation.h>

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
	PixelFormat m_format;
	PtrSize m_relativeOffset;

	Bool operator==(const VertexAttributeInfo& b) const
	{
		return m_bufferBinding == b.m_bufferBinding && m_format == b.m_format && m_relativeOffset == b.m_relativeOffset;
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
};

/// Model patch interface class. Its very important class and it binds the material with the mesh
class ModelPatch
{
public:
	ModelPatch(Model* model);

	~ModelPatch();

	const Material& getMaterial() const
	{
		return *m_mtl;
	}

	const Mesh& getMesh(const RenderingKey& key) const
	{
		return *m_meshes[key.m_lod];
	}

	const Model& getModel() const
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
		return m_meshes[0]->getBoundingShapeSub(subMeshId);
	}

	U32 getSubMeshesCount() const
	{
		return m_meshes[0]->getSubMeshesCount();
	}

	ANKI_USE_RESULT Error create(WeakArray<CString> meshFNames, const CString& mtlFName, ResourceManager* resources);

	/// Get information for multiDraw rendering. Given an array of submeshes that are visible return the correct indices
	/// offsets and counts.
	void getRenderingDataSub(const RenderingKey& key, WeakArray<U8> subMeshIndicesArray, ModelRenderingInfo& inf) const;

private:
	Model* m_model ANKI_DBG_NULLIFY;

	Array<MeshResourcePtr, MAX_LODS> m_meshes; ///< One for each LOD
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
class Model : public ResourceObject
{
public:
	Model(ResourceManager* manager);

	~Model();

	const DynamicArray<ModelPatch*>& getModelPatches() const
	{
		return m_modelPatches;
	}

	const Obb& getVisibilityShape() const
	{
		return m_visibilityShape;
	}

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

private:
	DynamicArray<ModelPatch*> m_modelPatches;
	Obb m_visibilityShape;
	SkeletonResourcePtr m_skeleton;
	DynamicArray<AnimationResourcePtr> m_animations;
};
/// @}

} // end namespace anki
