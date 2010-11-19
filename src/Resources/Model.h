#ifndef MODEL_H
#define MODEL_H

#include "Resource.h"
#include "RsrcPtr.h"


class Mesh;
class Material;
class Vao;


/// File format:
/// @code
/// subMeshes {
/// 	subMesh {
/// 		mesh <string>
/// 		material <string>
/// 		dpMaterial <string>
/// 	}
/// 	...
/// 	subMesh {
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
		RsrcPtr<Skeleton> skel;
		//RsrcPtr<SkelAnim> sAnim;
};


#endif
