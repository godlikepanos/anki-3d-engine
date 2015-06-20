// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/resource/ResourceObject.h"
#include "anki/Gr.h"
#include "anki/collision/Obb.h"
#include "anki/resource/RenderingKey.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/Animation.h"

namespace anki {

// Forward
class PhysicsCollisionShape;

/// @addtogroup resource
/// @{

/// Model patch interface class. Its very important class and it binds the
/// material with the mesh
class ModelPatch
{
public:
	ModelPatch(Model* model)
		: m_model(model)
	{}

	~ModelPatch();

	const Material& getMaterial() const
	{
		return *m_mtl;
	}

	const Mesh& getMesh(const RenderingKey& key) const
	{
		return *m_meshes[key.m_lod];
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

	ANKI_USE_RESULT Error create(
		SArray<CString> meshFNames,
		const CString& mtlFName,
		ResourceManager* resources);

	/// Get information for multiDraw rendering.
	/// Given an array of submeshes that are visible return the correct indices
	/// offsets and counts.
	void getRenderingDataSub(
		const RenderingKey& key,
		SArray<U8> subMeshIndicesArray,
		BufferPtr& vertBuff,
		BufferPtr& indexBuff,
		PipelinePtr& ppline,
		Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesCountArray,
		Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesOffsetArray,
		U32& drawcallCount) const;

private:
	Model* m_model;

	Array<MeshResourcePtr, MAX_LODS> m_meshes; ///< One for each LOD
	U8 m_meshCount = 0;
	MaterialResourcePtr m_mtl;

	mutable Array3d<PipelinePtr, U(Pass::COUNT), MAX_LODS, 2> m_pplines;

	/// Return the maximum number of LODs
	U getLodCount() const;

	PipelinePtr getPipeline(const RenderingKey& key) const;

	void computePipelineInitializer(
		const RenderingKey& key, PipelineInitializer& pinit) const;
};

/// Model is an entity that acts as a container for other resources. Models are
/// all the non static objects in a map.
///
/// XML file format:
/// @code
/// <model>
/// 	<modelPatches>
/// 		<modelPatch>
/// 			[<mesh>path/to/mesh.mesh</mesh>
///				[<mesh1>path/to/mesh_lod_1.mesh</mesh1>]
///				[<mesh2>path/to/mesh_lod_2.mesh</mesh2>]] |
/// 			[<bucketMesh>path/to/mesh.bmesh</bucketMesh>
///				[<bucketMesh1>path/to/mesh_lod_1.bmesh</bucketMesh1>]
///				[<bucketMesh2>path/to/mesh_lod_2.bmesh</bucketMesh2>]]
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
class Model: public ResourceObject
{
public:
	Model(ResourceManager* manager)
		: ResourceObject(manager)
	{}

	~Model();

	const DArray<ModelPatch*>& getModelPatches() const
	{
		return m_modelPatches;
	}

	const Obb& getVisibilityShape() const
	{
		return m_visibilityShape;
	}

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

private:
	DArray<ModelPatch*> m_modelPatches;
	Obb m_visibilityShape;
	SkeletonResourcePtr m_skeleton;
	DArray<AnimationResourcePtr> m_animations;
};
/// @}

} // end namespace anki

