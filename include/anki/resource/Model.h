#ifndef ANKI_RESOURCE_MODEL_H
#define ANKI_RESOURCE_MODEL_H

#include "anki/resource/Resource.h"
#include "anki/gl/Vao.h"
#include "anki/collision/Obb.h"
#include "anki/resource/PassLevelKey.h"
#include "anki/resource/Mesh.h"
#include "anki/util/Vector.h"

namespace anki {

// Forward
class ShaderProgram;

/// Model patch interface class. Its very important class and it binds the
/// material with the mesh
class ModelPatchBase
{
public:
	/// VAOs container
	typedef Vector<Vao> VaosContainer;

	virtual ~ModelPatchBase()
	{}

	const Material& getMaterial() const
	{
		ANKI_ASSERT(modelPatchProtected.mtl);
		return *modelPatchProtected.mtl;
	}

	const MeshBase& getMeshBase(const PassLevelKey& key) const
	{
		ANKI_ASSERT(key.level < modelPatchProtected.meshes.size());
		return *modelPatchProtected.meshes[key.level];
	}

	U32 getMeshesCount() const
	{
		return modelPatchProtected.meshes.size();
	}

	const Obb& getBoundingShape() const
	{
		PassLevelKey key(0, 0);
		return getMeshBase(key).getBoundingShape();
	}

	const Obb& getBoundingShapeSub(U32 subMeshId) const
	{
		PassLevelKey key(0, 0);
		return getMeshBase(key).getBoundingShapeSub(subMeshId);
	}

	U32 getSubMeshesCount() const
	{
		PassLevelKey key(0, 0);
		return getMeshBase(key).getSubMeshesCount();
	}

	/// Given a pass lod key retrieve variables useful for rendering
	void getRenderingData(const PassLevelKey& key, const Vao*& vao,
		const ShaderProgram*& prog, U32& indicesCount) const;

	/// Get information for multiDraw rendering
	void getRenderingDataSub(const PassLevelKey& key, U64 subMeshesMask,
		const Vao*& vao, const ShaderProgram*& prog,
		U32* indicesCountArray, U32* indicesOffsetArray, U32& primcount) const;

protected:
	struct
	{
		/// Array [lod][pass]
		VaosContainer vaos;
		Material* mtl = nullptr;
		Vector<MeshBase*> meshes;
	} modelPatchProtected;

	/// Create VAOs using a material and a mesh. It writes a VaosContainer and
	/// a hash map
	void create();

private:
	/// Called by @a createVaos multiple times to create and populate a single
	/// VAO
	static void createVao(
		const ShaderProgram &prog,
		const MeshBase& mesh,
		Vao& vao);
};

/// Its a chunk of a model. Its very important class and it binds the material
/// with the mesh
template<typename MeshResourcePointerType>
class ModelPatch: public ModelPatchBase
{
public:
	/// Map to get the VAO given a PassLod key
	typedef PassLevelHashMap<Vao> PassLevelToVaoMap;

	ModelPatch(const char* meshFNames[], U32 meshesCount,
		const char* mtlFName)
	{
		// Load
		ANKI_ASSERT(meshesCount > 0);
		meshes.resize(meshesCount);
		modelPatchProtected.meshes.resize(meshesCount);

		for(U32 i = 0; i < meshesCount; i++)
		{
			meshes[i].load(meshFNames[i]);
			modelPatchProtected.meshes[i] = meshes[i].get();

			if(i > 0 && !meshes[i]->isCompatible(*meshes[i - 1]))
			{
				throw ANKI_EXCEPTION("Meshes not compatible");
			}
		}

		mtl.load(mtlFName);
		modelPatchProtected.mtl = mtl.get();

		/// Create VAOs
		create();
	}

	~ModelPatch()
	{}

private:
	Vector<MeshResourcePointerType> meshes; ///< The geometries
	MaterialResourcePointer mtl; ///< Material
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
	typedef Vector<ModelPatchBase*> ModelPatchesContainer;

	Model()
	{}
	~Model();

	/// @name Accessors
	/// @{
	const ModelPatchesContainer& getModelPatches() const
	{
		return modelPatches;
	}

	const Obb& getVisibilityShape() const
	{
		return visibilityShape;
	}
	/// @}

	void load(const char* filename);

private:
	/// The vector of ModelPatch
	ModelPatchesContainer modelPatches;
	Obb visibilityShape;
};

} // end namespace

#endif
