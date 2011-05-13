#include "ScriptingCommon.h"
#include "Input.h"


WRAP(Input)
{
	class_<Input, noncopyable>("Input", no_init)
		.def("warpMouse", (bool (Input::*)() const)(&Input::warpMouse))
		.def("setWarpMouse", &Input::setWarpMouse)
	;
}
