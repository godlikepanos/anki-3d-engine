#include "ScriptingCommon.h"
#include "Scene.h"


WRAP(Scene)
{
	class_<Scene, noncopyable>("Scene", no_init)
		.def("setAmbientCol", &Scene::setAmbientCol)
		.def("getAmbientCol", (const Vec3& (Scene::*)() const)(&Scene::getAmbientCol),
		     return_value_policy<reference_existing_object>())

		.def("getCameras", (Scene::Types<Camera>::Container& (Scene::*)())(&Scene::getCameras),
		     return_value_policy<reference_existing_object>())
	;
}


WRAP_SINGLETON(SceneSingleton)
