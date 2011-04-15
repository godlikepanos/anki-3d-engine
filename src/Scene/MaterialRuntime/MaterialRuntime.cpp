#include <boost/foreach.hpp>
#include "MaterialRuntime.h"
#include "Material.h"
#include "Exception.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
MaterialRuntime::MaterialRuntime(const Material& mtl_):
	mtl(mtl_)
{
	MaterialProps& me = *this;
	const MaterialProps& he = mtl.accessMaterialPropsBaseClass();
	me = he;

	BOOST_FOREACH(const MtlUserDefinedVar& udv, mtl.getUserDefinedVars())
	{
		MaterialRuntimeUserDefinedVar* udvr = new MaterialRuntimeUserDefinedVar(udv);
		userDefVars.push_back(udvr);
		userDefVarsHashMap[udvr->getName().c_str()] = udvr;
	}
}


//======================================================================================================================
// getUserDefinedVarByName                                                                                             =
//======================================================================================================================
MaterialRuntimeUserDefinedVar& MaterialRuntime::getUserDefinedVarByName(const char* name)
{
	CharPtrHashMap<MaterialRuntimeUserDefinedVar*>::iterator it = userDefVarsHashMap.find(name);
	if(it == userDefVarsHashMap.end())
	{
		throw EXCEPTION("Cannot get user defined variable with name \"" + name + '\"');
	}
	return *(it->second);
}


//======================================================================================================================
// getUserDefinedVarByName                                                                                             =
//======================================================================================================================
const MaterialRuntimeUserDefinedVar& MaterialRuntime::getUserDefinedVarByName(const char* name) const
{
	CharPtrHashMap<MaterialRuntimeUserDefinedVar*>::const_iterator it = userDefVarsHashMap.find(name);
	if(it == userDefVarsHashMap.end())
	{
		throw EXCEPTION("Cannot get user defined variable with name \"" + name + '\"');
	}
	return *(it->second);
}
