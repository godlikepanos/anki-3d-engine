#include "anki/script/Common.h"
#include "anki/renderer/Dbg.h"

namespace anki {

static bool getEnabled(Dbg* self)
{
	return self->getEnabled();
}

static void setEnabled(Dbg* self, bool x)
{
	return self->setEnabled(x);
}

ANKI_SCRIPT_WRAP(Dbg)
{
	ANKI_LUA_CLASS_BEGIN(lb, Dbg)
		ANKI_LUA_CONSTRUCTOR(Renderer*)
		ANKI_LUA_FUNCTION_AS_METHOD("getEnabled", &getEnabled)
		ANKI_LUA_FUNCTION_AS_METHOD("setEnabled", &setEnabled)
	ANKI_LUA_CLASS_END()
}

} // end namespace anki

