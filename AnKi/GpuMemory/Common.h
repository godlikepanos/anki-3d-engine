// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Logger.h>

namespace anki {

#define ANKI_GPUMEM_LOGI(...) ANKI_LOG("GPUM", kNormal, __VA_ARGS__)
#define ANKI_GPUMEM_LOGE(...) ANKI_LOG("GPUM", kError, __VA_ARGS__)
#define ANKI_GPUMEM_LOGW(...) ANKI_LOG("GPUM", kWarning, __VA_ARGS__)
#define ANKI_GPUMEM_LOGF(...) ANKI_LOG("GPUM", kFatal, __VA_ARGS__)
#define ANKI_GPUMEM_LOGV(...) ANKI_LOG("GPUM", kVerbose, __VA_ARGS__)

} // end namespace anki
