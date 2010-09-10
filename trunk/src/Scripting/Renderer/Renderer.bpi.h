
/*class_<Renderer::Pps::Hdr, noncopyable>("Hdr", no_init)
	.def("getBlurringDist", &Renderer::Pps::Hdr::getBlurringDist, return_value_policy<reference_existing_object>())
;*/

/*class_<Renderer::Pps, noncopyable>("Pps", no_init)
;*/

class_<Renderer, noncopyable>("Renderer", no_init)
	.def("getFramesNum", &Renderer::getFramesNum)
;
