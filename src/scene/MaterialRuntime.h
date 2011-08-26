#ifndef MATERIAL_RUNTIME_MATERIAL_RUNTIME_H
#define MATERIAL_RUNTIME_MATERIAL_RUNTIME_H

#include "rsrc/MaterialProperties.h"
#include "util/Accessors.h"
#include "util/ConstCharPtrHashMap.h"
#include <boost/ptr_container/ptr_vector.hpp>


class Material;
class MaterialRuntimeVariable;


/// One layer above material resource
class MaterialRuntime: public MaterialProperties
{
	public:
		/// A type
		typedef boost::ptr_vector<MaterialRuntimeVariable>
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

		/// Find a material runtime variable. On failure it throws an exception
		/// @param[in] name The name of the var
		/// @return It returns a MaterialRuntimeUserDefinedVar
		/// @exception Exception
		MaterialRuntimeVariable& findVariableByName(const char* name);

		/// The const version of getUserDefinedVarByName
		/// @see getUserDefinedVarByName
		const MaterialRuntimeVariable& findVariableByName(
			const char* name) const;

	private:
		const Material& mtl; ///< The resource
		VariablesContainer vars;
		ConstCharPtrHashMap<MaterialRuntimeVariable*>::Type varNameToVar;
};


#endif
