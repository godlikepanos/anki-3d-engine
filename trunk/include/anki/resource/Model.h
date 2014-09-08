// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_MODEL_H
#define ANKI_RESOURCE_MODEL_H

#include "anki/resource/ResourceManager.h"
#include "anki/Gl.h"
#include "anki/collision/Obb.h"
#include "anki/resource/RenderingKey.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/Animation.h"
#include "anki/util/Vector.h"

namespace anki {

/// Model patch interface class. Its very important class and it binds the
/// material with the mesh
class ModelPatchBase
{
public:
	virtual ~ModelPatchBase()
	{}

	ModelPatchBase(ResourceAllocator<U8>& alloc)
	:	m_vertJobs(alloc),
		m_meshes(alloc)
	{}

	const Material& getMaterial() const
	{
		ANKI_ASSERT(m_mtl);
		return *m_mtl;
	}

	const Mesh& getMesh(const RenderingKey& key) const
	{
		ANKI_ASSERT(key.m_lod < m_meshes.size());
		return *m_meshes[key.m_lod];
	}

	U32 getMeshesCount() const
	{
		return m_meshes.size();
	}

	const Obb& getBoundingShape() const
	{
		RenderingKey key(Pass::COLOR, 0, false);
		return getMesh(key).getBoundingShape();
	}

	const Obb& getBoundingShapeSub(U32 subMeshId) const
	{
		RenderingKey key(Pass::COLOR, 0, false);
		return getMesh(key).getBoundingShapeSub(subMeshId);
	}

	U32 getSubMeshesCount() const
	{
		RenderingKey key(Pass::COLOR, 0, false);
		return getMesh(key).getSubMeshesCount();
	}

	/// Get information for multiDraw rendering.
	/// Given an array of submeshes that are visible return the correct indices
	/// offsets and counts
	void getRenderingDataSub(
		const RenderingKey& key, 
		GlCommandBufferHandle& vertJobs,
		GlProgramPipelineHandle& ppline,
		const U8* subMeshIndicesArray, 
		U32 subMeshIndicesCount,
		Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesCountArray,
		Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesOffsetArray, 
		U32& drawcallCount) const;

protected:
	/// Array [lod][pass]
	ResourceVector<GlCommandBufferHandle> m_vertJobs;
	Material* m_mtl = nullptr;
	ResourceVector<Mesh*> m_meshes; ///< One for each LOD

	/// Create vertex descriptors using a material and a mesh
	void create(GlDevice* gl);

private:
	/// Called by @a create multiple times to create and populate a single
	/// vertex descriptor
	static void createVertexDesc(
		const GlProgramHandle& prog,
		const Mesh& mesh,
		GlCommandBufferHandle& vertexJobs);

	/// Return the maximum number of LODs
	U getLodsCount() const;

	U getVertexDescIdx(const RenderingKey& key) const;
};

/// Its a chunk of a model. Its very important class and it binds the material
/// with the mesh
template<typename MeshResourcePointerType>
class ModelPatch: public ModelPatchBase
{
public:
	/// Accepts a number of mesh filenames, one for each LOD
	ModelPatch(
		CString meshFNames[], 
		U32 meshesCount,
		const CString& mtlFName, 
		ResourceManager* resources);

	~ModelPatch()
	{}

private:
	ResourceVector<MeshResourcePointerType> m_meshResources; ///< Geometries
	MaterialResourcePointer m_mtlResource; ///< Material
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
/// 	[<collisionShape>
/// 		<type>sphere | box | mesh</type>
/// 		<value>...</value>
/// 	</collisionShape>]
/// </model>
/// @endcode
///
/// Requirements:
/// - If the materials need texture coords then mesh should have them
/// - The skeleton and skelAnims are optional
/// - Its an error to have skelAnims without skeleton
class Model
{
public:
	Model();
	~Model();

	/// @name Accessors
	/// @{
	const ResourceVector<ModelPatchBase*>& getModelPatches() const
	{
		return m_modelPatches;
	}

	const Obb& getVisibilityShape() const
	{
		return m_visibilityShape;
	}
	/// @}

	void load(const CString& filename, ResourceInitializer& init);

private:
	/// The vector of ModelPatch
	ResourceVector<ModelPatchBase*> m_modelPatches;
	Obb m_visibilityShape;
	SkeletonResourcePointer m_skeleton;
	ResourceVector<AnimationResourcePointer> m_animations;
};

} // end namespace anki

#endif
