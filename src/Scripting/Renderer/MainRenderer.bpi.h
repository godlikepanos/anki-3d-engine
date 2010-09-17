
class_<MainRenderer, bases<Renderer>, noncopyable>("MainRenderer", no_init)
	.def_readonly("dbg", &MainRenderer::dbg)
;
