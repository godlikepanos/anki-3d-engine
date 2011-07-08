#include "ScriptingCommon.h"
#include "Renderer/Dbg.h"


WRAP(Dbg)
{
	class_<R::Dbg, noncopyable>("Dbg", no_init)
		.def("isEnabled", (bool (R::Dbg::*)() const)(&R::Dbg::isEnabled))
		.def("setEnabled", &R::Dbg::setEnabled)
	;
}
