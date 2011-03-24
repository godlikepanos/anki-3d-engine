#ifndef MATERIAL_RUNTIME_H
#define MATERIAL_RUNTIME_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "MtlUserDefinedVarRuntime.h"
#include "Properties.h"
#include "CharPtrHashMap.h"


class Material;


/// One layer above material resource
class MaterialRuntime
{
	public:
		MaterialRuntime(const Material& mtl);

		/// @name Accessors
		/// @{
		GETTER_RW(boost::ptr_vector<MtlUserDefinedVarRuntime>, userDefVars, getUserDefinedVars)

		/// Find MtlUserDefinedVarRuntime variable. On failure it throws an exception
		/// @param[in] name The name of the var
		/// @return It returns a MtlUserDefinedVarRuntime
		/// @exception Exception
		MtlUserDefinedVarRuntime& getUserDefinedVarByName(const char* name);

		/// The const version of getUserDefinedVarByName
		/// @see getUserDefinedVarByName
		const MtlUserDefinedVarRuntime& getUserDefinedVarByName(const char* name) const;

		const Material& getMaterial() const {return mtl;}

		/// @todo
		template<typename Type>
		void setUserDefVar(const char* name, const Type& value);
		/// @}

	private:
		const Material& mtl; ///< The resource
		boost::ptr_vector<MtlUserDefinedVarRuntime> userDefVars;
		CharPtrHashMap<MtlUserDefinedVarRuntime*> userDefVarsHashMap; ///< For fast finding the variables
};


template<typename Type>
void MaterialRuntime::setUserDefVar(const char* name, const Type& value)
{
	getUserDefinedVarByName(name).get<Type>() = value;
}


#endif
