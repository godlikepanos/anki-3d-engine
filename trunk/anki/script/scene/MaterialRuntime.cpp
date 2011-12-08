#include "anki/script/Common.h"
#include "anki/scene/MaterialRuntime.h"


WRAP(MaterialRuntimeVariable)
{
	class_<MaterialRuntimeVariable, noncopyable>("MaterialRuntimeVariable",
		no_init)
		/*.def("setValue", (void (MaterialRuntimeVariable::*)(const float&))
			(&MaterialRuntimeVariable::setValue))
		.def("setValue", (void (MaterialRuntimeVariable::*)(const Vec2&))
			(&MaterialRuntimeVariable::setValue))
		.def("setValue", (void (MaterialRuntimeVariable::*)(const Vec3&))
			(&MaterialRuntimeVariable::setValue))*/
	;
}


WRAP(MaterialRuntime)
{
	class_<MaterialRuntime, noncopyable>("MaterialRuntime", no_init)
		/*.def("getVariables",
			(MaterialRuntime::VariablesContainer&
			(MaterialRuntime::*)())(&MaterialRuntime::getVariables),
			return_value_policy<reference_existing_object>())

		.def("getWireframe", (bool (MaterialRuntime::*)() const)(
			&MaterialRuntime::getWireframe))
		.def("setWireframe", &MaterialRuntime::setWireframe)*/
	;
}
