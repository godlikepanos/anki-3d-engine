// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/MemoryPool.h>

namespace anki {

#define ANKI_WIND_LOGI(...) ANKI_LOG("WIND", kNormal, __VA_ARGS__)
#define ANKI_WIND_LOGE(...) ANKI_LOG("WIND", kError, __VA_ARGS__)
#define ANKI_WIND_LOGW(...) ANKI_LOG("WIND", kWarning, __VA_ARGS__)
#define ANKI_WIND_LOGF(...) ANKI_LOG("WIND", kFatal, __VA_ARGS__)

class WindowMemoryPool : public HeapMemoryPool, public MakeSingleton<WindowMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	WindowMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "WindowMemPool")
	{
	}

	~WindowMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Window, WindowMemoryPool)

} // end namespace anki
