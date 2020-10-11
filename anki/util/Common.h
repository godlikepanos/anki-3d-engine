// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Config.h>

namespace anki
{

#define ANKI_UTIL_LOGI(...) ANKI_LOG("UTIL", NORMAL, __VA_ARGS__)
#define ANKI_UTIL_LOGE(...) ANKI_LOG("UTIL", ERROR, __VA_ARGS__)
#define ANKI_UTIL_LOGW(...) ANKI_LOG("UTIL", WARNING, __VA_ARGS__)
#define ANKI_UTIL_LOGF(...) ANKI_LOG("UTIL", FATAL, __VA_ARGS__)

} // end namespace anki
