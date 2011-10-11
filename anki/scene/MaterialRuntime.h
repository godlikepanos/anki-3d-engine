#ifndef ANKI_SCENE_MATERIAL_RUNTIME_H
#define ANKI_SCENE_MATERIAL_RUNTIME_H

#include "anki/resource/MaterialProperties.h"
#include "anki/util/ConstCharPtrHashMap.h"
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
		bool getCastShadow() const {return castsShadowFlag;}
		bool& getCastShadow() {return castsShadowFlag;}
		void setCastShadow(bool x) {castsShadowFlag = x;}

		bool getRenderInBledingStage() const {return renderInBlendingStageFlag;}
		bool& getRenderInBledingStage() {return renderInBlendingStageFlag;}
		void setRenderInBledingStage(bool x) {renderInBlendingStageFlag = x;}

		int getBlendingSFactor() const {return blendingSfactor;}
		int& getBlendingSFactor() {return blendingSfactor;}
		void setBlendingSFactor(int x) {blendingSfactor = x;}

		int getBlendingDFactor() const {return blendingDfactor;}
		int& getBlendingDFactor() {return blendingDfactor;}
		void setBlendingDFactor(int x) {blendingDfactor = x;}

		bool getDepthTesting() const {return depthTesting;}
		bool& getDepthTesting() {return depthTesting;}
		void setDepthTesting(bool x) {depthTesting = x;}

		bool getWireframe() const {return wireframe;}
		bool& getWireframe() {return wireframe;}
		void setWireframe(bool x) {wireframe = x;}

		const VariablesContainer& getVariables() const {return vars;}
		VariablesContainer& getVariables() {return vars;}

		const Material& getMaterial() const {return mtl;}
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
