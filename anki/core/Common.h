// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/Config.h>

namespace anki
{

#define ANKI_CORE_LOGI(...) ANKI_LOG("CORE", NORMAL, __VA_ARGS__)
#define ANKI_CORE_LOGE(...) ANKI_LOG("CORE", ERROR, __VA_ARGS__)
#define ANKI_CORE_LOGW(...) ANKI_LOG("CORE", WARNING, __VA_ARGS__)
#define ANKI_CORE_LOGF(...) ANKI_LOG("CORE", FATAL, __VA_ARGS__)

} // end namespace anki
