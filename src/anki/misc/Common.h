// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Config.h>

namespace anki
{

#define ANKI_MISC_LOGI(...) ANKI_LOG("MISC", NORMAL, __VA_ARGS__)
#define ANKI_MISC_LOGE(...) ANKI_LOG("MISC", ERROR, __VA_ARGS__)
#define ANKI_MISC_LOGW(...) ANKI_LOG("MISC", WARNING, __VA_ARGS__)
#define ANKI_MISC_LOGF(...) ANKI_LOG("MISC", FATAL, __VA_ARGS__)

} // end namespace anki
