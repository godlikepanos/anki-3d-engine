// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Common.h>
#include <AnKi/Core/CVarSet.h>

namespace anki {

/// There is no need to ask for a fence or a semaphore to be waited for more than 10 seconds. The GPU will timeout anyway.
constexpr Second kMaxFenceOrSemaphoreWaitTime = 120.0;

constexpr U32 kMaxQueriesPerQueryChunk = 64;

extern NumericCVar<U32> g_maxBindlessSampledTextureCountCVar;

} // end namespace anki
