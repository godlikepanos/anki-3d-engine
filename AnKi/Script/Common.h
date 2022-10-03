// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Ptr.h>

namespace anki {

// Forward
class LuaBinder;
class ScriptManager;
class ScriptEnvironment;

#define ANKI_SCRIPT_LOGI(...) ANKI_LOG("SCRI", kNormal, __VA_ARGS__)
#define ANKI_SCRIPT_LOGE(...) ANKI_LOG("SCRI", kError, __VA_ARGS__)
#define ANKI_SCRIPT_LOGW(...) ANKI_LOG("SCRI", kWarning, __VA_ARGS__)
#define ANKI_SCRIPT_LOGF(...) ANKI_LOG("SCRI", kFatal, __VA_ARGS__)

} // end namespace anki
