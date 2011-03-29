#include "ScriptingCommon.h"
#include "MaterialRuntime.h"


WRAP(MaterialRuntime)
{
	class_<MaterialRuntime, noncopyable>("MaterialRuntime", no_init)
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const float&))(&MaterialRuntime::setUserDefVarValue))
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const Vec2&))(&MaterialRuntime::setUserDefVarValue))
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const Vec3&))(&MaterialRuntime::setUserDefVarValue))
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const MtlUserDefinedVar::Fai&))(&MaterialRuntime::setUserDefVarValue))

		.def("isWireframeEnabled", (bool (MaterialRuntime::*)() const)(&MaterialRuntime::isWireframeEnabled))
		.def("setWireframeEnabled", &MaterialRuntime::setWireframeEnabled)
	;
}
