
class_<App, noncopyable>("App", no_init)
	.def("getScene", &App::getScene, return_value_policy<reference_existing_object>())
	.def("getMainRenderer", &App::getMainRenderer, return_value_policy<reference_existing_object>())
	.def("quit", &App::quit)
;
