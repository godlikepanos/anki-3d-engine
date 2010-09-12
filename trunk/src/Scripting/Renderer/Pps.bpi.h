
class_<Pps, noncopyable>("Pps", no_init)
	.def_readonly("hdr", &Pps::hdr)
;
