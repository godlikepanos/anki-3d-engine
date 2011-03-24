#include "ScriptingCommon.h"
#include "Camera.h"


WRAP(Camera)
{
	class_<Camera, noncopyable>("Camera", no_init)
		.def("setFovX", &Camera::setFovX)
		.def("getFovX", &Camera::getFovX)
	;
}

