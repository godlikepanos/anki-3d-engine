
class_<Scene, noncopyable>("Scene", no_init)
	.def("setAmbientCol", &Scene::setAmbientCol)
	.def("getAmbientCol", &Scene::getAmbientCol, return_value_policy<reference_existing_object>())
;
