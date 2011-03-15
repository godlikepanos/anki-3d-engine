#include <boost/foreach.hpp>
#include "MaterialRuntime.h"
#include "Material.h"


//======================================================================================================================
//                                                                                                                     =
//======================================================================================================================
MaterialRuntime::MaterialRuntime(const Material& mtl_):
	mtl(mtl_)
{
	BOOST_FOREACH(const MtlUserDefinedVar& udv, mtl.getUserDefinedVars())
	{
		userDefVars.push_back(new MtlUserDefinedVarRuntime(udv));
	}
}
