// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Config.h>

namespace anki
{

#define ANKI_SCRIPT_LOGI(...) ANKI_LOG("SCRI", NORMAL, __VA_ARGS__)
#define ANKI_SCRIPT_LOGE(...) ANKI_LOG("SCRI", ERROR, __VA_ARGS__)
#define ANKI_SCRIPT_LOGW(...) ANKI_LOG("SCRI", WARNING, __VA_ARGS__)
#define ANKI_SCRIPT_LOGF(...) ANKI_LOG("SCRI", FATAL, __VA_ARGS__)

} // end namespace anki
