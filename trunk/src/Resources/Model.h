#ifndef MODEL_H
#define MODEL_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "Resource.h"
#include "RsrcPtr.h"
#include "Object.h"
#include "Vao.h"


class Mesh;
class Material;
class Skeleton;
class SkelAnim;
class Scanner;


/// Model is an entity that acts as a container for other resources. Models are all the non static objects in a map.
///
/// XML file format:
/// @code
/// <model>
/// 	<subModels>
/// 		<subModel>
/// 			<mesh>path/to/mesh.mesh</mesh>
/// 			<material>path/to/material.mtl</material>
/// 			<dpMaterial>path/to/dp.mtl</dpMaterial>
/// 		</subModel>
/// 		...
/// 		<subModel>...</subModel>
/// 	</subModels>
/// 	<skeleton>path/to/skeleton.skel</skeleton>
/// 	<skelAnims>
/// 		<skelAnim>path/to/anim0.sanim</skelAnim>
/// 		...
/// 		<skelAnim>...</skelAnim>
/// 	</skelAnims>
/// </model>
/// @endcode
///
/// Requirements:
/// - If the materials need texture coords or/and vertex weights then mesh should have them
/// - The skeleton and skelAnims are optional
/// - Its an error to have skelAnims without skeleton
class Model: public Resource
{
	public:
		/// This is basically a container around mesh and materials. It also has the VAOs.
		class SubModel
		{
			public:
				SubModel(){}

				/// Load the resources
				void load(const char* meshFName, const char* mtlFName, const char* dpMtlFName);

				/// Creates a VAO for an individual SubModel
				/// @param[in] material Needed for the shader program uniform variables
				/// @param[in] mesh For providing the VBOs
				/// @param[in,out] subModel For setting a parent to the vao
				/// @param[out] vao The output
				static void createVao(const Material& material, const Mesh& mesh, Vao& vao);

				/// @name Accessors
				/// @{
				const Mesh& getMesh() const {return *mesh;}
				const Material& getMaterial() const {return *material;}
				const Material& getDpMaterial() const {return *dpMaterial;}
				const Vao& getVao() const {return vao;}
				const Vao& getDpVao() const {return dpVao;}
				/// @}

				bool hasHwSkinning() const;

			private:
				RsrcPtr<Mesh> mesh; ///< The geometry
				RsrcPtr<Material> material; ///< Material for MS and BS
				RsrcPtr<Material> dpMaterial; ///< Material for depth passes
				Vao vao; ///< Normal VAO for MS and BS
				Vao dpVao; ///< Depth pass VAO for SM and EarlyZ
		};

		Model(): Resource(RT_MODEL) {}

		void load(const char* filename);

		/// @name Accessors
		/// @{
		const boost::ptr_vector<SubModel>& getSubModels() const {return subModels;}
		const Skeleton& getSkeleton() const;
		const Vec<RsrcPtr<SkelAnim> >& getSkelAnims() const {return skelAnims;}
		/// @}

		bool hasSkeleton() const {return skeleton.get() != NULL;}

	private:
		boost::ptr_vector<SubModel> subModels; ///< The vector of SubModel
		RsrcPtr<Skeleton> skeleton; ///< The skeleton. It can be empty
		Vec<RsrcPtr<SkelAnim> > skelAnims; ///< The standard skeleton animations
};


inline const Skeleton& Model::getSkeleton() const
{
	RASSERT_THROW_EXCEPTION(!hasSkeleton());
	return *skeleton;
}


#endif
