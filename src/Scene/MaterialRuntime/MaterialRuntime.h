#ifndef MATERIAL_RUNTIME_H
#define MATERIAL_RUNTIME_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "MtlUserDefinedVarRuntime.h"
#include "Properties.h"
#include "CharPtrHashMap.h"


class Material;


/// @todo
class MaterialRuntime
{
	public:
		MaterialRuntime(const Material& mtl);

		/// @name Accessors
		/// @{
		GETTER_RW(boost::ptr_vector<MtlUserDefinedVarRuntime>, userDefVars, getUserDefVars)

		/// Find MtlUserDefinedVarRuntime variable. On failure it throws an exception
		/// @param[in] name The name of the var
		/// @return It returns a MtlUserDefinedVarRuntime
		/// @exception Exception
		MtlUserDefinedVarRuntime& getUserDefinedVarByName(const char* name);

		/// @see getUserDefinedVarByName
		const MtlUserDefinedVarRuntime& getUserDefinedVarByName(const char* name) const;
		/// @}

	private:
		const Material& mtl; ///< The resource
		boost::ptr_vector<MtlUserDefinedVarRuntime> userDefVars;
		CharPtrHashMap<MtlUserDefinedVarRuntime*> userDefVarsHashMap; ///< For fast finding the variables
};


#endif
