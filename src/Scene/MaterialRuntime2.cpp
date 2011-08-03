#include "MaterialRuntime2.h"
#include "Resources/Material2.h"
#include "UserMaterialVariableRuntime.h"
#include <boost/foreach.hpp>


//==============================================================================
// Constructor                                                                 =
//==============================================================================
MaterialRuntime2::MaterialRuntime2(const Material2& mtl_)
:	mtl(mtl_)
{
	// Copy props
	MaterialProperties& me = *this;
	const MaterialProperties& he = mtl.accessMaterialPropertiesBaseClass();
	me = he;

	// Create vars
	BOOST_FOREACH(const UserMaterialVariable* var, mtl.getUserVariables())
	{
		UserMaterialVariableRuntime* varr =
			new UserMaterialVariableRuntime(*var);
		vars.push_back(varr);
		varNameToVar[varr->getUserMaterialVariable().getName().c_str()] = varr;
	}
}


//==============================================================================
// Destructor                                                                  =
//==============================================================================
MaterialRuntime2::~MaterialRuntime2()
{}


//==============================================================================
// findVariableByName                                                          =
//==============================================================================
UserMaterialVariableRuntime& MaterialRuntime2::findVariableByName(
	const char* name)
{
	ConstCharPtrHashMap<UserMaterialVariableRuntime*>::Type::iterator it =
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
const UserMaterialVariableRuntime& MaterialRuntime2::findVariableByName(
	const char* name) const
{
	ConstCharPtrHashMap<UserMaterialVariableRuntime*>::Type::const_iterator
		it = varNameToVar.find(name);
	if(it == varNameToVar.end())
	{
		throw EXCEPTION("Cannot get user defined variable with name \"" +
			name + '\"');
	}
	return *(it->second);
}
