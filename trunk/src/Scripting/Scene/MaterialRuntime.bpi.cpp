#include "ScriptingCommon.h"
#include "Scene/MaterialRuntime.h"


WRAP(MaterialRuntime)
{
	/*class_<MaterialRuntime::MaterialRuntimeUserDefinedVarContainer, noncopyable>("MaterialRuntime::MaterialRuntimeUserDefinedVarContainer", no_init)
		.def(vector_indexing_suite<MaterialRuntime::MaterialRuntimeUserDefinedVarContainer, true>())
	;*/
	//WRAP_CONTAINER(MaterialRuntime::MaterialRuntimeUserDefinedVarContainer)

	class_<MaterialRuntime, noncopyable>("MaterialRuntime", no_init)
		.def("getUserDefinedVars",
		     (boost::ptr_vector<MaterialRuntimeUserDefinedVar>& (MaterialRuntime::*)())(&MaterialRuntime::getUserDefinedVars),
		     return_value_policy<reference_existing_object>())

		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const float&))(&MaterialRuntime::setUserDefVarValue))
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const Vec2&))(&MaterialRuntime::setUserDefVarValue))
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const Vec3&))(&MaterialRuntime::setUserDefVarValue))
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const MtlUserDefinedVar::Fai&))(&MaterialRuntime::setUserDefVarValue))

		.def("isWireframeEnabled", (bool (MaterialRuntime::*)() const)(&MaterialRuntime::isWireframeEnabled))
		.def("setWireframeEnabled", &MaterialRuntime::setWireframeEnabled)
	;
}
