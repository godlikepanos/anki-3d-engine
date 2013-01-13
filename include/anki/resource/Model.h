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
	/// Map to get the VAO given a PassLod key
	typedef PassLevelHashMap<Vao*> PassLevelToVaoMap;

	virtual ~ModelPatchBase()
	{}

	virtual const MeshBase& getMeshBase(const PassLevelKey& key) const = 0;
	virtual U32 getMeshesCount() const = 0;
	virtual const Material& getMaterial() const = 0;

	const Obb& getBoundingShape() const
	{
		PassLevelKey key(0, 0);
		return getMeshBase(key).getBoundingShape();
	}

	/// Given a pass lod key retrieve variables useful for rendering
	void getRenderingData(const PassLevelKey& key, const Vao*& vao,
		const ShaderProgram*& prog, U32& indicesCount) const;

protected:
	/// Array [lod][pass]
	VaosContainer vaos;

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
class ModelPatch: public ModelPatchBase
{
public:
	/// Map to get the VAO given a PassLod key
	typedef PassLevelHashMap<Vao> PassLevelToVaoMap;

	ModelPatch(const char* meshFNames[], U32 meshesCount, const char* mtlFName);
	~ModelPatch();

	/// @name Accessors
	/// @{

	/// Implements ModelPatchBase::getMeshBase
	const MeshBase& getMeshBase(const PassLevelKey& key) const
	{
		return *meshes[key.level];
	}

	/// Implements ModelPatchBase::getMeshesCount
	U32 getMeshesCount() const
	{
		return meshes.size();
	}

	/// Implements ModelPatchBase::getMaterial
	const Material& getMaterial() const
	{
		return *mtl;
	}
	/// @}

private:
	Vector<MeshResourcePointer> meshes; ///< The geometries
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
/// 			<mesh>path/to/mesh.mesh</mesh>
///				[<mesh1>path/to/mesh_lod_1.mesh</mesh1>]
///				[<mesh2>path/to/mesh_lod_2.mesh</mesh2>]
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
	typedef PtrVector<ModelPatch> ModelPatchesContainer;

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
