// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Logger.h>
#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/HashMap.h>

namespace anki {

/// @addtogroup importer
/// @{

#define ANKI_IMPORTER_LOGI(...) ANKI_LOG("IMPR", kNormal, __VA_ARGS__)
#define ANKI_IMPORTER_LOGV(...) ANKI_LOG("IMPR", kVerbose, __VA_ARGS__)
#define ANKI_IMPORTER_LOGE(...) ANKI_LOG("IMPR", kError, __VA_ARGS__)
#define ANKI_IMPORTER_LOGW(...) ANKI_LOG("IMPR", kWarning, __VA_ARGS__)
#define ANKI_IMPORTER_LOGF(...) ANKI_LOG("IMPR", kFatal, __VA_ARGS__)

using ImporterString = BaseString<MemoryPoolPtrWrapper<BaseMemoryPool>>;
using ImporterStringList = BaseStringList<MemoryPoolPtrWrapper<BaseMemoryPool>>;

template<typename TKey, typename TValue>
using ImporterHashMap = HashMap<TKey, TValue, DefaultHasher<TKey>, MemoryPoolPtrWrapper<BaseMemoryPool>>;

template<typename T>
using ImporterDynamicArray = DynamicArray<T, MemoryPoolPtrWrapper<BaseMemoryPool>, U32>;

template<typename T>
using ImporterDynamicArrayLarge = DynamicArray<T, MemoryPoolPtrWrapper<BaseMemoryPool>, PtrSize>;

template<typename T>
using ImporterList = List<T, MemoryPoolPtrWrapper<BaseMemoryPool>>;

/// @}

} // end namespace anki
