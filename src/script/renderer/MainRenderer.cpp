#include "anki/script/Common.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/renderer/Dbg.h"
#include "anki/renderer/Deformer.h"

namespace anki {

static Dbg& getDbg(MainRenderer* self)
{
	return self->getDbg();
}

static Pps& getPps(MainRenderer* self)
{
	return self->getPps();
}

ANKI_SCRIPT_WRAP(MainRenderer)
{
	ANKI_LUA_CLASS_BEGIN(lb, MainRenderer)
		ANKI_LUA_CONSTRUCTOR()
		ANKI_LUA_FUNCTION_AS_METHOD("getDbg", &getDbg)
		ANKI_LUA_FUNCTION_AS_METHOD("getPps", &getPps)
	ANKI_LUA_CLASS_END()
}

ANKI_SCRIPT_WRAP_SINGLETON(MainRendererSingleton)

} // end namespace anki

