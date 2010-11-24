#ifndef MODEL_H
#define MODEL_H

#include "Resource.h"
#include "RsrcPtr.h"
#include "Object.h"


class Mesh;
class Material;
class Vao;
class Skeleton;
class SkelAnim;
class Scanner;


/// Model is an entity that acts as a container for other resources. Models are all the non static objects in a map.
///
/// File format:
/// @code
/// subModels {
/// 	subModel {
/// 		mesh <string>
/// 		material <string>
/// 		dpMaterial <string>
/// 	}
/// 	...
/// 	subModel {
/// 		...
/// 	}
/// }
///
/// skeleton <string>
///
/// skelAnims {
/// 	<string>
/// 	...
/// 	<string>
/// }
/// @endcode
class Model: public Resource
{
	public:
		/// This is basicaly a container around mesh and materials. It also has the VAOs.
		class SubModel: public Object
		{
			friend class Model;

			public:
				SubModel(): Object(NULL) {}

				/// @name Accessors
				/// @{
				const Mesh& getMesh() const {return *mesh;}
				const Material& getMaterial() const {return *material;}
				const Material& getDpMaterial() const {return *dpMaterial;}
				const Vao& getVao() const {return *vao;}
				const Vao& getDpVao() const {return *dpVao;}
				/// @}

			private:
				RsrcPtr<Mesh> mesh; ///< The geometry
				RsrcPtr<Material> material; ///< Material for MS ans BS
				RsrcPtr<Material> dpMaterial; ///< Material for depth passes
				Vao* vao; ///< Normal VAO for MS ans BS
				Vao* dpVao; ///< Depth pass VAO for SM and EarlyZ
		};

		Model(): Resource(RT_MODEL) {}

		void load(const char* filename);

		/// @name Accessors
		/// @{
		const Vec<SubModel>& getSubModels() const {return subModels;}
		const Skeleton& getSkeleton() const;
		/// @}

		bool hasSkeleton() const {return skeleton.get() != NULL;}

	private:
		Vec<SubModel> subModels; ///< The vector of submodels
		RsrcPtr<Skeleton> skeleton; ///< The skeleton. It can be empty
		Vec<RsrcPtr<SkelAnim> > skelAnims; ///< The standard skeleton animations

		/// Parses a submodel from after the "subModel" until the closing bracket
		void parseSubModel(Scanner& scanner);

		/// Creates VAOs for an individual submodel
		void createVao(const Material& mtl, const Mesh& mesh, SubModel& subModel, Vao*& vao);
};


inline const Skeleton& Model::getSkeleton() const
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return *skeleton;
}


#endif
