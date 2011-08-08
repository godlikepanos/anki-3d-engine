#ifndef MATERIAL_RUNTIME_2_H
#define MATERIAL_RUNTIME_2_H

#include "Resources/Material.h"
#include "Util/Accessors.h"
#include "Util/ConstCharPtrHashMap.h"
#include <boost/ptr_container/ptr_vector.hpp>


class UserVariableRuntime;
class Material;


/// One layer above material resource
class MaterialRuntime: private MaterialProperties
{
	public:
		/// A type
		typedef boost::ptr_vector<UserVariableRuntime>
			VariablesContainer;

		/// @name Constructors & destructors
		/// @{
		MaterialRuntime(const Material& mtl);
		~MaterialRuntime();
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
		GETTER_R(Material, mtl, getMaterial);
		/// @}

		/// Find MaterialRuntimeUserDefinedVar variable. On failure it throws
		/// an exception
		/// @param[in] name The name of the var
		/// @return It returns a MaterialRuntimeUserDefinedVar
		/// @exception Exception
		UserVariableRuntime& findVariableByName(
			const char* name);

		/// The const version of getUserDefinedVarByName
		/// @see getUserDefinedVarByName
		const UserVariableRuntime& findVariableByName(
			const char* name) const;

		bool isBlendingEnabled() const;

	private:
		const Material& mtl; ///< The resource
		VariablesContainer vars;
		ConstCharPtrHashMap<UserVariableRuntime*>::Type
			varNameToVar;
};


inline bool MaterialRuntime::isBlendingEnabled() const
{
	return blendingSfactor != GL_ONE || blendingDfactor != GL_ZERO;
}


#endif
