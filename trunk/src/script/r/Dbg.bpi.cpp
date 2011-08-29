#include "ScriptingCommon.h"
#include "r/Dbg.h"


WRAP(Dbg)
{
	using namespace r;

	class_<Dbg, noncopyable>("Dbg", no_init)
		.def("isEnabled", (bool (Dbg::*)() const)(&Dbg::isEnabled))
		.def("setEnabled", &Dbg::setEnabled)
	;
}
