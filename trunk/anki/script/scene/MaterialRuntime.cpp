#include "anki/script/Common.h"
#include "anki/scene/MaterialRuntime.h"


WRAP(MaterialRuntime)
{
	class_<MaterialRuntime, noncopyable>("MaterialRuntime", no_init)
		.def("getVariables",
			(MaterialRuntime::VariablesContainer&
			(MaterialRuntime::*)())(&MaterialRuntime::getVariables),
			return_value_policy<reference_existing_object>())

		.def("getWireframe", (bool (MaterialRuntime::*)() const)(
			&MaterialRuntime::getWireframe))
		.def("setWireframe", &MaterialRuntime::setWireframe)
	;
}
