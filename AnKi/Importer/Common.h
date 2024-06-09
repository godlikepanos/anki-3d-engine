// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

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

class ImporterMemoryPool : public HeapMemoryPool, public MakeSingleton<ImporterMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	ImporterMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "ImporterMemPool")
	{
	}

	~ImporterMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(Importer, ImporterMemoryPool)
/// @}

} // end namespace anki
