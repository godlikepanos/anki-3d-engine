#ifndef ANKI_RESOURCE_MODEL_H
#define ANKI_RESOURCE_MODEL_H

#include "anki/resource/Resource.h"
#include "anki/gl/Vao.h"
#include "anki/collision/Obb.h"
#include "anki/resource/PassLevelKey.h"
#include "anki/resource/Mesh.h"
#include <boost/ptr_container/ptr_vector.hpp>


namespace anki {


/// Its a chunk of a model. Its very important class and it binds the material
/// with the mesh
class ModelPatch
{
public:
	typedef boost::ptr_vector<Vao> VaosContainer;
	typedef PassLevelHashMap<Vao*>::Type PassLevelToVaoHashMap;
	typedef boost::array<const Vbo*, Mesh::VBOS_NUM> VboArray;

	ModelPatch(const char* meshFName, const char* mtlFName);
	~ModelPatch();

	/// @name Accessors
	/// @{
	const Mesh& getMesh() const
	{
		return *mesh;
	}

	const Material& getMaterial() const
	{
		return *mtl;
	}
	/// @}

	bool supportsHwSkinning() const;

	const Vao& getVao(const PassLevelKey& key) const
	{
		return *vaosHashMap.at(key);
	}

	static void createVaos(const Material& mtl,
		const VboArray& vbos,
		VaosContainer& vaos,
		PassLevelToVaoHashMap& vaosHashMap);

private:
	MeshResourcePointer mesh; ///< The geometry
	MaterialResourcePointer mtl; ///< Material

	VaosContainer vaos;
	PassLevelToVaoHashMap vaosHashMap;

	static Vao* createVao(const Material& mtl,
		const VboArray& vbos,
		const PassLevelKey& key);
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
	typedef boost::ptr_vector<ModelPatch> ModelPatchesContainer;

	void load(const char* filename);

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

private:
	/// The vector of ModelPatch
	ModelPatchesContainer modelPatches;
	Obb visibilityShape;
};


} // end namespace


#endif
