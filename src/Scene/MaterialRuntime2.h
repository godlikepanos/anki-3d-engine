#ifndef MATERIAL_RUNTIME_2_H
#define MATERIAL_RUNTIME_2_H

#include "Resources/Material2.h"
#include "Util/Accessors.h"
#include "Util/ConstCharPtrHashMap.h"
#include <boost/ptr_container/ptr_vector.hpp>


class UserMaterialVariableRuntime;
class Material2;


/// One layer above material resource
class MaterialRuntime2: private MaterialProperties
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
		GETTER_SETTER_BY_VAL(bool, castsShadowFlag, castsShadow, setCastShadow)
		GETTER_SETTER_BY_VAL(bool, renderInBlendingStageFlag,
			rendersInBlendingStage, setRendersInBlendingStage)
		GETTER_SETTER_BY_VAL(int, blendingSfactor, getBlendingSfactor,
			setBlendingSFactor)
		GETTER_SETTER_BY_VAL(int, blendingDfactor, getBlendingDfactor,
			setBlendingDFactor)
		GETTER_SETTER_BY_VAL(bool, depthTesting, isDepthTestingEnabled,
			setDepthTestingEnabled)
		GETTER_SETTER_BY_VAL(bool, wireframe, isWireframeEnabled,
			setWireframeEnabled)

		GETTER_RW(VariablesContainer, vars, getVariables)
		GETTER_R(Material2, mtl, getMaterial);
		/// @}

		/// Find MaterialRuntimeUserDefinedVar variable. On failure it throws
		/// an exception
		/// @param[in] name The name of the var
		/// @return It returns a MaterialRuntimeUserDefinedVar
		/// @exception Exception
		UserMaterialVariableRuntime& findVariableByName(
			const char* name);

		/// The const version of getUserDefinedVarByName
		/// @see getUserDefinedVarByName
		const UserMaterialVariableRuntime& findVariableByName(
			const char* name) const;

		bool isBlendingEnabled() const;

	private:
		const Material2& mtl; ///< The resource
		VariablesContainer vars;
		ConstCharPtrHashMap<UserMaterialVariableRuntime*>::Type
			varNameToVar;
};


inline bool MaterialRuntime2::isBlendingEnabled() const
{
	return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;
}


#endif
