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
	BOOST_FOREACH(const MtlUserDefinedVar& udv, mtl.getUserDefinedVars())
	{
		MtlUserDefinedVarRuntime* udvr = new MtlUserDefinedVarRuntime(udv);
		userDefVars.push_back(udvr);
		userDefVarsHashMap[udvr->getName().c_str()] = udvr;
	}
}


//======================================================================================================================
// getUserDefinedVarByName                                                                                             =
//======================================================================================================================
MtlUserDefinedVarRuntime& MaterialRuntime::getUserDefinedVarByName(const char* name)
{
	CharPtrHashMap<MtlUserDefinedVarRuntime*>::iterator it = userDefVarsHashMap.find(name);
	if(it == userDefVarsHashMap.end())
	{
		throw EXCEPTION("Cannot get user defined variable with name \"" + name + '\"');
	}
	return *(it->second);
}


//======================================================================================================================
// getUserDefinedVarByName                                                                                             =
//======================================================================================================================
const MtlUserDefinedVarRuntime& MaterialRuntime::getUserDefinedVarByName(const char* name) const
{
	CharPtrHashMap<MtlUserDefinedVarRuntime*>::const_iterator it = userDefVarsHashMap.find(name);
	if(it == userDefVarsHashMap.end())
	{
		throw EXCEPTION("Cannot get user defined variable with name \"" + name + '\"');
	}
	return *(it->second);
}
