// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/ui/Common.h>

namespace anki
{

// Forward
class ResourceManager;
class GrManager;
class StagingGpuMemoryManager;

/// @addtogroup ui
/// @{

/// UI manager.
class UiManager
{
public:
	UiManager();

	~UiManager();

	ANKI_USE_RESULT Error init(
		HeapAllocator<U8> alloc, ResourceManager* resources, GrManager* gr, StagingGpuMemoryManager* gpuMem);

	UiAllocator getAllocator() const
	{
		return m_alloc;
	}

	ResourceManager& getResourceManager()
	{
		ANKI_ASSERT(m_resources);
		return *m_resources;
	}

	GrManager& getGrManager()
	{
		ANKI_ASSERT(m_gr);
		return *m_gr;
	}

	StagingGpuMemoryManager& getStagingGpuMemoryManager()
	{
		ANKI_ASSERT(m_gpuMem);
		return *m_gpuMem;
	}

	/// Create a new UI object.
	template<typename T, typename... Args>
	ANKI_USE_RESULT Error newInstance(IntrusivePtr<T>& ptr, Args&&... args)
	{
		ptr.reset(m_alloc.newInstance<T>(this));
		return ptr->init(args...);
	}

private:
	UiAllocator m_alloc;
	ResourceManager* m_resources = nullptr;
	GrManager* m_gr = nullptr;
	StagingGpuMemoryManager* m_gpuMem = nullptr;
};
/// @}

} // end namespace anki
