#ifndef ANKI_RESOURCE_MODEL_H
#define ANKI_RESOURCE_MODEL_H

#include "anki/resource/Resource.h"
#include "anki/gl/Vao.h"
#include "anki/collision/Obb.h"
#include "anki/resource/PassLevelKey.h"
#include "anki/resource/Mesh.h"
#include "anki/util/Vector.h"

namespace anki {

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

	virtual const MeshBase& getMeshBase() const = 0;
	virtual const Material& getMaterial() const = 0;

	const Vao& getVao(const PassLevelKey& key) const
	{
		PassLevelToVaoMap::const_iterator it = vaosMap.find(key);
		ANKI_ASSERT(it != vaosMap.end());
		return *(*it);
	}

	/// Allias to MeshBase::getIndicesCount()
	U32 getIndecesCount(const U32 lod) const
	{
		return getMeshBase().getIndicesCount(lod);
	}

protected:
	VaosContainer vaos;
	PassLevelToVaoMap vaosMap;

	void create()
	{
		createVaos(getMaterial(), getMeshBase(), vaos, vaosMap);
	}

	/// Create VAOs using a material and a mesh. It writes a VaosContainer and
	/// a hash map
	static void createVaos(const Material& mtl,
		const MeshBase& mesh,
		VaosContainer& vaos,
		PassLevelToVaoMap& vaosMap);

private:
	/// Called by @a createVaos multiple times to create and populate a single
	/// VAO
	static Vao* createNewVao(const Material& mtl,
		const MeshBase& mesh,
		const PassLevelKey& key);
};

/// Its a chunk of a model. Its very important class and it binds the material
/// with the mesh
class ModelPatch: public ModelPatchBase
{
public:
	/// Map to get the VAO given a PassLod key
	typedef PassLevelHashMap<Vao> PassLevelToVaoMap;

	ModelPatch(const char* meshFName, const char* mtlFName);
	~ModelPatch();

	/// @name Accessors
	/// @{

	/// Implements ModelPatchBase::getMeshBase
	const MeshBase& getMeshBase() const
	{
		return *mesh;
	}

	/// Implements ModelPatchBase::getMaterial
	const Material& getMaterial() const
	{
		return *mtl;
	}
	/// @}

private:
	MeshResourcePointer mesh; ///< The geometry
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
