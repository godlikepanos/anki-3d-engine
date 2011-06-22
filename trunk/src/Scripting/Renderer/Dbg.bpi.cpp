#include "ScriptingCommon.h"
#include "Dbg.h"


WRAP(Dbg)
{
	class_<Dbg, noncopyable>("Dbg", no_init)
		.def("isEnabled", (bool (Dbg::*)() const)(&Dbg::isEnabled))
		.def("setEnabled", &Dbg::setEnabled)
	;
}
