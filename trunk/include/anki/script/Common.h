/// @file
/// This file is included by all the .cpp files that wrap something

#ifndef ANKI_SCRIPT_COMMON_H
#define ANKI_SCRIPT_COMMON_H

#include "anki/script/LuaBinder.h"

/// Wrap a class
#define ANKI_SCRIPT_WRAP(x) \
	void ankiScriptWrap##x(LuaBinder& lb)

/// XXX
#define ANKI_SCRIPT_CALL_WRAP(x) \
	extern void ankiScriptWrap##x(LuaBinder&); \
	ankiScriptWrap##x(*this);

#endif
