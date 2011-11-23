#ifndef ANKI_RESOURCE_MODEL_PATCH_H
#define ANKI_RESOURCE_MODEL_PATCH_H

#include "anki/resource/Resource.h"
#include "anki/resource/PassLevelKey.h"
#include "anki/resource/Mesh.h"
#include "anki/gl/Vao.h"
#include <boost/ptr_container/ptr_vector.hpp>


namespace anki {


class Material;


/// Its one part of the many used in the Model class. Its basically a container
/// for a Mesh and it's Material
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


} // end namespace


#endif
