#include "anki/scene/MaterialRuntime.h"
#include "anki/resource/Material.h"
#include "anki/scene/MaterialRuntimeVariable.h"
#include <boost/foreach.hpp>


namespace anki {


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialRuntime::MaterialRuntime(const Material& mtl_)
:	mtl(mtl_)
{
	// Copy props
	MaterialProperties& me = *this;
	const MaterialProperties& he = mtl.getBaseClass();
	me = he;

	// Create vars
	BOOST_FOREACH(const MaterialUserVariable* var, mtl.getUserVariables())
	{
		MaterialRuntimeVariable* varr = new MaterialRuntimeVariable(*var);
		vars.push_back(varr);
		varNameToVar[varr->getMaterialUserVariable().getName()] = varr;
	}
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
MaterialRuntime::~MaterialRuntime()
{}


//==============================================================================
// findVariableByName                                                          =
//==============================================================================
MaterialRuntimeVariable& MaterialRuntime::findVariableByName(
	const char* name)
{
	ConstCharPtrHashMap<MaterialRuntimeVariable*>::Type::iterator it =
		varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw ANKI_EXCEPTION("Cannot get user defined variable with name \"" +
			name + '\"');
	}
	return *(it->second);
}


//==============================================================================
// findVariableByName                                                          =
//==============================================================================
const MaterialRuntimeVariable& MaterialRuntime::findVariableByName(
	const char* name) const
{
	ConstCharPtrHashMap<MaterialRuntimeVariable*>::Type::const_iterator
		it = varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw ANKI_EXCEPTION("Cannot get user defined variable with name \"" +
			name + '\"');
	}
	return *(it->second);
}


} // end namespace
