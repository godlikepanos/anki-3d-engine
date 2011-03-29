#ifndef MATERIAL_RUNTIME_H
#define MATERIAL_RUNTIME_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "MtlUserDefinedVarRuntime.h"
#include "Properties.h"
#include "CharPtrHashMap.h"
#include "Material.h"



/// One layer above material resource
class MaterialRuntime: private MaterialProps
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

		/// Find var and set its value
		template<typename Type>
		void setUserDefVarValue(const char* name, const Type& value);

		/// Find var and set its value
		template<typename Type>
		const Type& getUserDefVarValue(const char* name) const;

		template<typename Type>
		Type& getUserDefVarValue(const char* name);

		GETTER_SETTER_BY_VAL(bool, castsShadowFlag, castsShadow, setCastsShadow)
		GETTER_SETTER_BY_VAL(bool, renderInBlendingStageFlag, renderInBlendingStage, setRenderInBlendingStage)
		GETTER_SETTER_BY_VAL(int, blendingSfactor, getBlendingSfactor, setBlendingSfactor)
		GETTER_SETTER_BY_VAL(int, blendingDfactor, getBlendingDfactor, setBlendingDfactor)
		GETTER_SETTER_BY_VAL(bool, depthTesting, isDepthTestingEnabled, setDepthTestingEnabled)
		GETTER_SETTER_BY_VAL(bool, wireframe, isWireframeEnabled, setWireframeEnabled)

		const SProgAttribVar* getStdAttribVar(Material::StdAttribVars id) const {return mtl.getStdAttribVar(id);}
		const SProgUniVar* getStdUniVar(Material::StdUniVars id) const {return mtl.getStdUniVar(id);}
		const ShaderProg& getShaderProg() const {return mtl.getShaderProg();}
		/// @}

		bool isBlendingEnabled() const {return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;}

	private:
		const Material& mtl; ///< The resource
		boost::ptr_vector<MtlUserDefinedVarRuntime> userDefVars;
		CharPtrHashMap<MtlUserDefinedVarRuntime*> userDefVarsHashMap; ///< For fast finding the variables
};


template<typename Type>
const Type& MaterialRuntime::getUserDefVarValue(const char* name) const
{
	return getUserDefinedVarByName(name).get<Type>();
}


template<typename Type>
Type& MaterialRuntime::getUserDefVarValue(const char* name)
{
	return getUserDefinedVarByName(name).get<Type>();
}


template<typename Type>
void MaterialRuntime::setUserDefVarValue(const char* name, const Type& value)
{
	getUserDefinedVarByName(name).set<Type>(value);
}


#endif
