// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Config.h>

namespace anki {

#define ANKI_UTIL_LOGI(...) ANKI_LOG("UTIL", kNormal, __VA_ARGS__)
#define ANKI_UTIL_LOGE(...) ANKI_LOG("UTIL", kError, __VA_ARGS__)
#define ANKI_UTIL_LOGW(...) ANKI_LOG("UTIL", kWarning, __VA_ARGS__)
#define ANKI_UTIL_LOGF(...) ANKI_LOG("UTIL", kFatal, __VA_ARGS__)

} // end namespace anki
