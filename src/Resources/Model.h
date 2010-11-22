#ifndef MODEL_H
#define MODEL_H

#include "Resource.h"
#include "RsrcPtr.h"


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
/// stdAnims {
/// 	<string>
/// 	...
/// 	<string>
/// }
/// @endcode
class Model: public Resource
{
	public:
		///
		class SubModel
		{
			friend class Model;

			public:
				/// @name Accessors
				/// @{
				const Mesh& getMesh() const {return *mesh;}
				const Material& getMtl() const {return *mtl;}
				const Material& getDpMtl() const {return *dpMtl;}
				const Vao& getVao() const {return *normVao;}
				const Vao& getDpVao() const {return *dpVao;}
				/// @}

			private:
				RsrcPtr<Mesh> mesh; ///< The geometry
				RsrcPtr<Material> mtl; ///< Material for MS ans BS
				RsrcPtr<Material> dpMtl; ///< Material for depth passes
				Vao* normVao; ///< Normal VAO for MS ans BS
				Vao* dpVao; ///< Depth pass VAO for SM and EarlyZ
		};

		Model(): Resource(RT_MODEL) {}

		void load(const char* filename);

	private:
		Vec<SubModel> subModels;
		RsrcPtr<Skeleton> skel; ///< The skeleton. It can be empty
		Vec<RsrcPtr<SkelAnim> > sAnims; ///< The standard skeleton animations

		void parseSubModel(Scanner& scanner);
};


#endif
