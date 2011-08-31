#include "Common.h"
#include "r/Dbg.h"


WRAP(Dbg)
{
	class_<Dbg, noncopyable>("Dbg", no_init)
		.def("isEnabled", (bool (Dbg::*)() const)(&Dbg::isEnabled))
		.def("setEnabled", &Dbg::setEnabled)
	;
}
