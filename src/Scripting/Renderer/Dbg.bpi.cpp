#include "ScriptingCommon.h"
#include "Dbg.h"


WRAP(Dbg)
{
	class_<Dbg, noncopyable>("Dbg", no_init)
		.add_property("enabled", (bool (Dbg::*)() const)(&Dbg::isEnabled), &Dbg::setEnabled)
	;
}
