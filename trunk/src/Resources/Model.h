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
/// Text file format:
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
		/// This is basically a container around mesh and materials. It also has the VAOs.
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
				RsrcPtr<Material> material; ///< Material for MS and BS
				RsrcPtr<Material> dpMaterial; ///< Material for depth passes
				Vao* vao; ///< Normal VAO for MS and BS
				Vao* dpVao; ///< Depth pass VAO for SM and EarlyZ
		};

		Model(): Resource(RT_MODEL) {}

		void load(const char* filename);

		/// @name Accessors
		/// @{
		const Vec<SubModel>& getSubModels() const {return subModels;}
		const Skeleton& getSkeleton() const;
		const Vec<RsrcPtr<SkelAnim> >& getSkelAnims() const {return skelAnims;}
		/// @}

		bool hasSkeleton() const {return skeleton.get() != NULL;}

	private:
		Vec<SubModel> subModels; ///< The vector of SubModel
		RsrcPtr<Skeleton> skeleton; ///< The skeleton. It can be empty
		Vec<RsrcPtr<SkelAnim> > skelAnims; ///< The standard skeleton animations

		/// Parses a submodel from after the "subModel" until the closing bracket
		void parseSubModel(Scanner& scanner);

		/// Creates a VAO for an individual SubModel
		/// @param[in] material Needed for the shader program uniform variables
		/// @param[in] mesh For providing the VBOs
		/// @param[in,out] subModel For setting a parent to the vao
		/// @param[out] vao The output
		static void createVao(const Material& material, const Mesh& mesh, SubModel& subModel, Vao*& vao);
};


inline const Skeleton& Model::getSkeleton() const
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return *skeleton;
}


#endif
