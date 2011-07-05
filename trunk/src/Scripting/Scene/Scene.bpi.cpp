#include "ScriptingCommon.h"
#include "Scene/Scene.h"
#include "Scene/Camera.h"
#include "Scene/ModelNode.h"


WRAP(Scene)
{
	WRAP_CONTAINER(Scene::Types<Camera>::Container)
	WRAP_CONTAINER(Scene::Types<ModelNode>::Container)

	class_<Scene, noncopyable>("Scene", no_init)
		.def("setAmbientCol", &Scene::setAmbientCol)
		.def("getAmbientCol", (const Vec3& (Scene::*)() const)(&
			Scene::getAmbientCol),
			return_value_policy<reference_existing_object>())

		.def("getCameras", (Scene::Types<Camera>::Container& (Scene::*)())(&
			Scene::getCameras),
			return_value_policy<reference_existing_object>())

		.def("getModelNodes", (Scene::Types<ModelNode>::Container&
			(Scene::*)())(&Scene::getModelNodes),
			return_value_policy<reference_existing_object>())
	;
}
