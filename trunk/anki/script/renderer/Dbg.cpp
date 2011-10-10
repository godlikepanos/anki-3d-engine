#include "anki/script/ScriptCommon.h"
#include "anki/renderer/Dbg.h"


WRAP(Dbg)
{
	class_<Dbg, bases<SwitchableRenderingPass>, noncopyable>("Dbg", no_init)
	;
}
