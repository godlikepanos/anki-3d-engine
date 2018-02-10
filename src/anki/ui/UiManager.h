// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
class Input;

/// @addtogroup ui
/// @{

/// UI manager.
class UiManager
{
public:
	UiManager();

	~UiManager();

	ANKI_USE_RESULT Error init(AllocAlignedCallback allocCallback,
		void* allocCallbackUserData,
		ResourceManager* resources,
		GrManager* gr,
		StagingGpuMemoryManager* gpuMem,
		Input* input);

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

	const Input& getInput() const
	{
		ANKI_ASSERT(m_input);
		return *m_input;
	}

	/// Create a new UI object.
	template<typename T, typename Y, typename... Args>
	ANKI_USE_RESULT Error newInstance(IntrusivePtr<Y>& ptr, Args&&... args)
	{
		T* p = m_alloc.newInstance<T>(this);
		ptr.reset(static_cast<Y*>(p));
		return p->init(args...);
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
	Input* m_input = nullptr;
};
/// @}

} // end namespace anki
