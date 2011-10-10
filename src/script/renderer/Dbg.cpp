#include "ScriptCommon.h"
#include "renderer/Dbg.h"


WRAP(Dbg)
{
	class_<Dbg, bases<SwitchableRenderingPass>, noncopyable>("Dbg", no_init)
	;
}
