#ifndef MATERIAL_RUNTIME_H
#define MATERIAL_RUNTIME_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "MaterialRuntimeUserDefinedVar.h"
#include "Accessors.h"
#include "CharPtrHashMap.h"
#include "Material.h"



/// One layer above material resource
class MaterialRuntime: private MaterialProps
{
	public:
		/// A type
		typedef boost::ptr_vector<MaterialRuntimeUserDefinedVar> MaterialRuntimeUserDefinedVarContainer;

		/// The one and only contructor
		MaterialRuntime(const Material& mtl);

		/// @name Accessors
		/// @{
		GETTER_RW(MaterialRuntimeUserDefinedVarContainer, userDefVars, getUserDefinedVars)

		/// Find MaterialRuntimeUserDefinedVar variable. On failure it throws an exception
		/// @param[in] name The name of the var
		/// @return It returns a MaterialRuntimeUserDefinedVar
		/// @exception Exception
		MaterialRuntimeUserDefinedVar& getUserDefinedVarByName(const char* name);

		/// The const version of getUserDefinedVarByName
		/// @see getUserDefinedVarByName
		const MaterialRuntimeUserDefinedVar& getUserDefinedVarByName(const char* name) const;

		/// Get the resource
		const Material& getMaterial() const {return mtl;}

		/// Find var and set its value
		template<typename Type>
		void setUserDefVarValue(const char* name, const Type& value);

		/// Find var and set its value
		template<typename Type>
		const Type& getUserDefVarValue(const char* name) const;

		/// The non const version of getUserDefVarValue
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
		MaterialRuntimeUserDefinedVarContainer userDefVars;
		CharPtrHashMap<MaterialRuntimeUserDefinedVar*> userDefVarsHashMap; ///< For fast finding the variables
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
