#include "Common.h"
#include "scene/Camera.h"


WRAP(Camera)
{
	class_<Camera, bases<SceneNode>, noncopyable>("Camera", no_init)
		/*.def("setFovX", &Camera::setFovX)
		.def("getFovX", &Camera::getFovX)*/
	;
}

