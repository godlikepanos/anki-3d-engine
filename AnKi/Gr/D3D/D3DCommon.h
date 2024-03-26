// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>
#include <AnKi/Util/Logger.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <AnKi/Util/CleanupWindows.h>

namespace anki {

#define ANKI_D3D_LOGI(...) ANKI_LOG("D3D", kNormal, __VA_ARGS__)
#define ANKI_D3D_LOGE(...) ANKI_LOG("D3D", kError, __VA_ARGS__)
#define ANKI_D3D_LOGW(...) ANKI_LOG("D3D", kWarning, __VA_ARGS__)
#define ANKI_D3D_LOGF(...) ANKI_LOG("D3D", kFatal, __VA_ARGS__)
#define ANKI_D3D_LOGV(...) ANKI_LOG("D3D", kVerbose, __VA_ARGS__)

} // end namespace anki
