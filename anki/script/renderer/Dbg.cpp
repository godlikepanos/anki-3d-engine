#include "anki/script/Common.h"
#include "anki/renderer/Dbg.h"


WRAP(Dbg)
{
	class_<Dbg, bases<SwitchableRenderingPass>, noncopyable>("Dbg", no_init)
	;
}
