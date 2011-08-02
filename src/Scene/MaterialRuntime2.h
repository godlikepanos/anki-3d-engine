#ifndef MATERIAL_RUNTIME_2_H
#define MATERIAL_RUNTIME_2_H

#include "Resources/Material.h"
#include "Util/Accessors.h"
#include <boost/ptr_container/ptr_vector.hpp>


class UserMaterialVariableRuntime;


/// One layer above material resource
class MaterialRuntime2: private MaterialProps
{
	public:
		/// A type
		typedef boost::ptr_vector<UserMaterialVariableRuntime>
			VariablesContainer;

		/// @name Constructors & destructors
		/// @{
		MaterialRuntime2(const Material2& mtl);
		~MaterialRuntime2();
		/// @}

		/// @name Accessors
		/// @{
		GETTER_RW(VariablesContainer, vars, getUserMaterialVariablesRuntime)
		/// @}

	private:
		VariablesContainer vars;
};


#endif
