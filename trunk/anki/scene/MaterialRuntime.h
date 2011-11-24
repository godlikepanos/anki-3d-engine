#ifndef ANKI_SCENE_MATERIAL_RUNTIME_H
#define ANKI_SCENE_MATERIAL_RUNTIME_H

#include "anki/resource/MaterialProperties.h"
#include "anki/util/ConstCharPtrHashMap.h"
#include "anki/scene/MaterialRuntimeVariable.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/range/iterator_range.hpp>


namespace anki {


class Material;


/// One layer above material resource
class MaterialRuntime: public MaterialProperties
{
	public:
		/// A type
		typedef boost::ptr_vector<MaterialRuntimeVariable>
			VariablesContainer;

		typedef boost::iterator_range<VariablesContainer::const_iterator>
			ConstIteratorRange;
		typedef boost::iterator_range<VariablesContainer::iterator>
			MutableIteratorRange;

		typedef ConstCharPtrHashMap<MaterialRuntimeVariable*>::Type
			VariablesHashMap;

		/// @name Constructors & destructor
		/// @{
		MaterialRuntime(const Material& mtl);
		~MaterialRuntime();
		/// @}

		/// @name Accessors
		/// @{
		using MaterialProperties::getRenderingStage;
		uint& getRenderingStage()
		{
			return renderingStage;
		}
		void setRenderingStage(uint x)
		{
			renderingStage = x;
		}

		using MaterialProperties::getPasses;

		using MaterialProperties::getLevelsOfDetail;

		using MaterialProperties::getShadow;
		bool& getShadow()
		{
			return shadow;
		}
		void setShadow(bool x)
		{
			shadow = x;
		}

		using MaterialProperties::getBlendingSfactor;
		int& getBlendingSFactor()
		{
			return blendingSfactor;
		}
		void setBlendingSFactor(int x)
		{
			blendingSfactor = x;
		}

		using MaterialProperties::getBlendingDfactor;
		int& getBlendingDFactor()
		{
			return blendingDfactor;
		}
		void setBlendingDFactor(int x)
		{
			blendingDfactor = x;
		}

		using MaterialProperties::getDepthTesting;
		bool& getDepthTesting()
		{
			return depthTesting;
		}
		void setDepthTesting(bool x)
		{
			depthTesting = x;
		}

		using MaterialProperties::getWireframe;
		bool& getWireframe()
		{
			return wireframe;
		}
		void setWireframe(bool x)
		{
			wireframe = x;
		}

		ConstIteratorRange getVariables() const
		{
			return ConstIteratorRange(vars.begin(), vars.end());
		}
		MutableIteratorRange getVariables()
		{
			return MutableIteratorRange(vars.begin(), vars.end());
		}

		const Material& getMaterial() const
		{
			return mtl;
		}
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

		bool variableExists(const char* name) const
		{
			return varNameToVar.find(name) != varNameToVar.end();
		}

	private:
		const Material& mtl; ///< The resource
		VariablesContainer vars;
		VariablesHashMap varNameToVar;
};


} // end namespace


#endif
