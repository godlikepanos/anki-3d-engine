
class_<Dbg, noncopyable>("Dbg", no_init)
	.add_property("enabled", &Dbg::isEnabled, &Dbg::setEnabled)
;
