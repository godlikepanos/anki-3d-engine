
class_<Hdr, noncopyable>("Hdr", no_init)
	.add_property("blurringIterations", &Hdr::getBlurringIterations, &Hdr::setBlurringIterations)
	.add_property("blurringDist", &Hdr::getBlurringDist, &Hdr::setBlurringDist)
	.add_property("exposure", &Hdr::getExposure, &Hdr::setExposure)
;
