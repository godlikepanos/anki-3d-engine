#include "ScriptingCommon.h"
#include "MaterialRuntime.h"


WRAP(MaterialRuntime)
{
	class_<MaterialRuntime, noncopyable>("MaterialRuntime", no_init)
		.def("setUserDefVar", (void (MaterialRuntime::*)(const char*, const Vec3&))(&MaterialRuntime::setUserDefVar))
	;
}
