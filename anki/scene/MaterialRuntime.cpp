#include "anki/scene/MaterialRuntime.h"
#include "anki/resource/Material.h"
#include "anki/scene/MaterialRuntimeVariable.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
MaterialRuntime::MaterialRuntime(const Material& mtl_)
:	mtl(mtl_)
{
	// Copy props
	MaterialProperties& me = *this;
	const MaterialProperties& he = mtl.getBaseClass();
	me = he;

	// Create vars
	BOOST_FOREACH(const MaterialVariable& var, mtl.getVariables())
	{
		if(var.getShaderProgramVariableType() !=
			ShaderProgramVariable::T_UNIFORM)
		{
			continue;
		}

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
