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
class UiManager : public MakeSingleton<UiManager>
{
	template<typename>
	friend class MakeSingleton;

public:
	Error init(UiManagerInitInfo& initInfo);

	/// Create a new UI object.
	template<typename T, typename Y, typename... Args>
	Error newInstance(IntrusivePtr<Y, UiObjectDeleter<Y>>& ptr, Args&&... args)
	{
		T* p = anki::newInstance<T>(UiMemoryPool::getSingleton());
		ptr.reset(static_cast<Y*>(p));
		return p->init(args...);
	}

	/// Create a new UI object.
	template<typename T, typename... Args>
	Error newInstance(IntrusivePtr<T, UiObjectDeleter<T>>& ptr, Args&&... args)
	{
		ptr.reset(anki::newInstance<T>(UiMemoryPool::getSingleton()));
		return ptr->init(args...);
	}

	UiExternalSubsystems& getExternalSubsystems()
	{
		return m_subsystems;
	}

private:
	UiExternalSubsystems m_subsystems;

	UiManager();

	~UiManager();
};
/// @}

} // end namespace anki
