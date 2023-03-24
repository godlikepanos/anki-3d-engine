// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Config.h>
#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/MemoryPool.h>

namespace anki {

#define ANKI_CORE_LOGI(...) ANKI_LOG("CORE", kNormal, __VA_ARGS__)
#define ANKI_CORE_LOGE(...) ANKI_LOG("CORE", kError, __VA_ARGS__)
#define ANKI_CORE_LOGW(...) ANKI_LOG("CORE", kWarning, __VA_ARGS__)
#define ANKI_CORE_LOGF(...) ANKI_LOG("CORE", kFatal, __VA_ARGS__)

class CoreMemoryPool : public HeapMemoryPool, public MakeSingleton<CoreMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	CoreMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "CoreMemPool")
	{
	}

	~CoreMemoryPool() = default;
};

using CoreString = BaseStringRaii<SingletonMemoryPoolWrapper<CoreMemoryPool>>;

} // end namespace anki
