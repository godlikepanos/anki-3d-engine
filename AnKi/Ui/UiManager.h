// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Ui/Common.h>

namespace anki {

/// @addtogroup ui
/// @{

/// UI manager.
class UiManager : public MakeSingleton<UiManager>
{
	template<typename>
	friend class MakeSingleton;

public:
	Error init(AllocAlignedCallback allocCallback, void* allocCallbackData);

	/// Create a new UI object.
	template<typename T, typename Y, typename... Args>
	Error newInstance(IntrusivePtr<Y, UiObjectDeleter>& ptr, Args&&... args)
	{
		T* p = anki::newInstance<T>(UiMemoryPool::getSingleton());
		ptr.reset(static_cast<Y*>(p));
		return p->init(args...);
	}

	/// Create a new UI object.
	template<typename T, typename... Args>
	Error newInstance(IntrusivePtr<T, UiObjectDeleter>& ptr, Args&&... args)
	{
		ptr.reset(anki::newInstance<T>(UiMemoryPool::getSingleton()));
		return ptr->init(args...);
	}

private:
	UiManager();

	~UiManager();
};
/// @}

} // end namespace anki
