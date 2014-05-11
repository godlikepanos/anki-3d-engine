#ifndef ANKI_RESOURCE_MODEL_H
#define ANKI_RESOURCE_MODEL_H

#include "anki/resource/Resource.h"
#include "anki/Gl.h"
#include "anki/collision/Obb.h"
#include "anki/resource/RenderingKey.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/Material.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/Animation.h"
#include "anki/util/Vector.h"

class btCollisionShape;

namespace anki {

/// Model patch interface class. Its very important class and it binds the
/// material with the mesh
class ModelPatchBase
{
public:
	virtual ~ModelPatchBase()
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
		GlJobChainHandle& vertJobs,
		GlProgramPipelineHandle& ppline,
		const U8* subMeshIndicesArray, 
		U32 subMeshIndicesCount,
		Array<U32, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesCountArray,
		Array<PtrSize, ANKI_GL_MAX_SUB_DRAWCALLS>& indicesOffsetArray, 
		U32& drawcallCount) const;

protected:
	/// Array [lod][pass]
	Vector<GlJobChainHandle> m_vertJobs;
	Material* m_mtl = nullptr;
	Vector<Mesh*> m_meshes; ///< One for each LOD

	/// Create vertex descriptors using a material and a mesh
	void create();

private:
	/// Called by @a create multiple times to create and populate a single
	/// vertex descriptor
	static void createVertexDesc(
		const GlProgramHandle& prog,
		const Mesh& mesh,
		GlJobChainHandle& vertexJobs);

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
	ModelPatch(const char* meshFNames[], U32 meshesCount,
		const char* mtlFName);

	~ModelPatch()
	{}

private:
	Vector<MeshResourcePointerType> m_meshResources; ///< The geometries
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
	const Vector<ModelPatchBase*>& getModelPatches() const
	{
		return m_modelPatches;
	}

	const Obb& getVisibilityShape() const
	{
		return m_visibilityShape;
	}

	const btCollisionShape* getCollisionShape() const
	{
		return m_collShape.get();
	}
	/// @}

	void load(const char* filename);

private:
	/// The vector of ModelPatch
	Vector<ModelPatchBase*> m_modelPatches;
	Obb m_visibilityShape;
	SkeletonResourcePointer m_skeleton;
	Vector<AnimationResourcePointer> m_animations;
	std::unique_ptr<btCollisionShape> m_collShape;
};

} // end namespace anki

#endif
