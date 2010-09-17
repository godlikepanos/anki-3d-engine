
class_<Dbg, noncopyable>("Dbg", no_init)
	.add_property("enable", &Dbg::isEnabled, &Dbg::setEnabled)
;
