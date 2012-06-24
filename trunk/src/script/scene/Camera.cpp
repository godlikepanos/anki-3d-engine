#include "anki/script/Common.h"
#include "anki/scene/Camera.h"

ANKI_WRAP(Camera)
{
	class_<Camera, bases<SceneNode>, noncopyable>("Camera", no_init)
		/*.def("setFovX", &Camera::setFovX)
		.def("getFovX", &Camera::getFovX)*/
	;
}

