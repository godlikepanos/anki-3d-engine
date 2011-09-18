#include "ScriptCommon.h"
#include "r/Dbg.h"


WRAP(Dbg)
{
	class_<Dbg, bases<SwitchableRenderingPass>, noncopyable>("Dbg", no_init)
	;
}
