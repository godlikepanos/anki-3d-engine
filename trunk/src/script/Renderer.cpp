// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/script/Common.h"
#include "anki/renderer/MainRenderer.h"

namespace anki {

//==============================================================================
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
	ANKI_LUA_CLASS_BEGIN_NO_DESTRUCTOR(lb, Dbg)
		ANKI_LUA_FUNCTION_AS_METHOD("getEnabled", &getEnabled)
		ANKI_LUA_FUNCTION_AS_METHOD("setEnabled", &setEnabled)
	ANKI_LUA_CLASS_END()
}

//==============================================================================
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
		ANKI_LUA_FUNCTION_AS_METHOD("getDbg", &getDbg)
		ANKI_LUA_FUNCTION_AS_METHOD("getPps", &getPps)
	ANKI_LUA_CLASS_END()
}

} // end namespace anki


