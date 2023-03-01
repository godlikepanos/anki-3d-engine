// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui/Common.h>

namespace anki {

/// @addtogroup ui
/// @{

class UiManagerInitInfo : public UiExternalSubsystems
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;
};

/// UI manager.
class UiManager
{
	friend class UiObject;

public:
	UiManager();

	~UiManager();

	Error init(UiManagerInitInfo& initInfo);

	HeapMemoryPool& getMemoryPool() const
	{
		return m_pool;
	}

	/// Create a new UI object.
	template<typename T, typename Y, typename... Args>
	Error newInstance(IntrusivePtr<Y>& ptr, Args&&... args)
	{
		T* p = anki::newInstance<T>(m_pool, this);
		ptr.reset(static_cast<Y*>(p));
		return p->init(args...);
	}

	/// Create a new UI object.
	template<typename T, typename... Args>
	Error newInstance(IntrusivePtr<T>& ptr, Args&&... args)
	{
		ptr.reset(anki::newInstance<T>(m_pool, this));
		return ptr->init(args...);
	}

private:
	mutable HeapMemoryPool m_pool;
	UiExternalSubsystems m_subsystems;
};
/// @}

} // end namespace anki
