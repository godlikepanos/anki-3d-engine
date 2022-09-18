// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Logger.h>

namespace anki {

/// @addtogroup importer
/// @{

#define ANKI_IMPORTER_LOGI(...) ANKI_LOG("IMPR", kNormal, __VA_ARGS__)
#define ANKI_IMPORTER_LOGV(...) ANKI_LOG("IMPR", kVerbose, __VA_ARGS__)
#define ANKI_IMPORTER_LOGE(...) ANKI_LOG("IMPR", kError, __VA_ARGS__)
#define ANKI_IMPORTER_LOGW(...) ANKI_LOG("IMPR", kWarning, __VA_ARGS__)
#define ANKI_IMPORTER_LOGF(...) ANKI_LOG("IMPR", kFatal, __VA_ARGS__)

/// @}

} // end namespace anki
