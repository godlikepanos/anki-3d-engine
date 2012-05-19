#include "anki/script/Common.h"
#include "anki/input/Input.h"


WRAP(Input)
{
	class_<Input, noncopyable>("Input", no_init)
		.def("getWarpMouse", (bool (Input::*)() const)(&Input::getWarpMouse))
		.def("setWarpMouse", &Input::setWarpMouse)
	;
}
