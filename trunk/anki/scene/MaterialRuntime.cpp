#include "anki/scene/MaterialRuntime.h"
#include "anki/resource/Material.h"
#include "anki/resource/Texture.h"
#include "anki/resource/Resource.h"
#include "anki/resource/ShaderProgram.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
// MaterialRuntimeVariable                                                     =
//==============================================================================

//==============================================================================
MaterialRuntimeVariable::MaterialRuntimeVariable(const MaterialVariable& mv)
	: mvar(mv), buildinId(-1)
{}


//==============================================================================
MaterialRuntimeVariable::~MaterialRuntimeVariable()
{}


//==============================================================================
const MaterialRuntimeVariable::Variant&
	MaterialRuntimeVariable::getVariantConst() const
{
	return copyVariant ? *copyVariant : mvar.getVariant();
}


//==============================================================================
MaterialRuntimeVariable::Variant& MaterialRuntimeVariable::getVariantMutable()
{
	if(copyVariant.get() == NULL)
	{
		copyVariant.reset(new Variant(mvar.getVariant()));
	}

	return *copyVariant;
}


//==============================================================================
// MaterialRuntime                                                             =
//==============================================================================

//==============================================================================
MaterialRuntime::MaterialRuntime(const Material& mtl_)
	: mtl(mtl_)
{
	// Copy props
	MaterialProperties& me = *this;
	const MaterialProperties& he = mtl.getBaseClass();
	me = he;

	// Create vars
	BOOST_FOREACH(const MaterialVariable& var, mtl.getVariables())
	{
		MaterialRuntimeVariable* varr = new MaterialRuntimeVariable(var);
		vars.push_back(varr);
		varNameToVar[varr->getMaterialVariable().getName().c_str()] = varr;
	}
}


//==============================================================================
MaterialRuntime::~MaterialRuntime()
{}


//==============================================================================
MaterialRuntimeVariable& MaterialRuntime::findVariableByName(
	const char* name)
{
	VariablesHashMap::iterator it = varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw ANKI_EXCEPTION("Cannot get material runtime variable "
			"with name \"" + name + '\"');
	}
	return *(it->second);
}


//==============================================================================
const MaterialRuntimeVariable& MaterialRuntime::findVariableByName(
	const char* name) const
{
	VariablesHashMap::const_iterator it = varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw ANKI_EXCEPTION("Cannot get material runtime variable "
			"with name \"" + name + '\"');
	}
	return *(it->second);
}


} // end namespace
