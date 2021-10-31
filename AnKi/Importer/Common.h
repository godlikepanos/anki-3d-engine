// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Logger.h>

namespace anki {

/// @addtogroup importer
/// @{

#define ANKI_IMPORTER_LOGI(...) ANKI_LOG("IMPR", NORMAL, __VA_ARGS__)
#define ANKI_IMPORTER_LOGV(...) ANKI_LOG("IMPR", VERBOSE, __VA_ARGS__)
#define ANKI_IMPORTER_LOGE(...) ANKI_LOG("IMPR", ERROR, __VA_ARGS__)
#define ANKI_IMPORTER_LOGW(...) ANKI_LOG("IMPR", WARNING, __VA_ARGS__)
#define ANKI_IMPORTER_LOGF(...) ANKI_LOG("IMPR", FATAL, __VA_ARGS__)

/// @}

} // end namespace anki
