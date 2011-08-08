#include "MaterialRuntime.h"
#include "Resources/Material.h"
#include "UserVariableRuntime.h"
#include <boost/foreach.hpp>


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialRuntime::MaterialRuntime(const Material& mtl_)
:	mtl(mtl_)
{
	// Copy props
	MaterialProperties& me = *this;
	const MaterialProperties& he = mtl.accessMaterialPropertiesBaseClass();
	me = he;

	// Create vars
	BOOST_FOREACH(const UserVariable* var, mtl.getUserVariables())
	{
		UserVariableRuntime* varr =
			new UserVariableRuntime(*var);
		vars.push_back(varr);
		varNameToVar[varr->getUserVariable().getName().c_str()] = varr;
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
UserVariableRuntime& MaterialRuntime::findVariableByName(
	const char* name)
{
	ConstCharPtrHashMap<UserVariableRuntime*>::Type::iterator it =
		varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw EXCEPTION("Cannot get user defined variable with name \"" +
			name + '\"');
	}
	return *(it->second);
}


//==============================================================================
// findVariableByName                                                          =
//==============================================================================
const UserVariableRuntime& MaterialRuntime::findVariableByName(
	const char* name) const
{
	ConstCharPtrHashMap<UserVariableRuntime*>::Type::const_iterator
		it = varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw EXCEPTION("Cannot get user defined variable with name \"" +
			name + '\"');
	}
	return *(it->second);
}
