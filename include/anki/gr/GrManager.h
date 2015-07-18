// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/gr/Common.h"
#include "anki/util/String.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Manager initializer.
class GrManagerInitializer
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;

	MakeCurrentCallback m_makeCurrentCallback = nullptr;
	void* m_makeCurrentCallbackData = nullptr;
	void* m_ctx = nullptr;

	SwapBuffersCallback m_swapBuffersCallback = nullptr;
	void* m_swapBuffersCallbackData = nullptr;

	CString m_cacheDirectory;
	Bool m_registerDebugMessages = false;
};

/// The graphics manager, owner of all graphics objects.
class GrManager
{
	friend class GrManagerImpl;

public:
	using Initializer = GrManagerInitializer;

	/// Default constructor
	GrManager();

	~GrManager();

	/// Create.
	ANKI_USE_RESULT Error create(Initializer& init);

	/// Swap buffers
	void swapBuffers();

	/// Create a new graphics object.
	template<typename T, typename... Args>
	IntrusivePtr<T> newInstance(Args&&... args);

	/// @privatesection
	/// @{
	GrAllocator<U8>& getAllocator()
	{
		return m_alloc;
	}

	GrManagerImpl& getImplementation()
	{
		return *m_impl;
	}

	const GrManagerImpl& getImplementation() const
	{
		return *m_impl;
	}

	CString getCacheDirectory() const
	{
		return m_cacheDir.toCString();
	}
	/// @}

private:
	UniquePtr<GrManagerImpl> m_impl;
	String m_cacheDir;
	GrAllocator<U8> m_alloc; ///< Keep it last to deleted last
};

template<typename T, typename... Args>
IntrusivePtr<T> GrManager::newInstance(Args&&... args)
{
	IntrusivePtr<T> ptr(m_alloc.newInstance<T>(this));
	ptr->create(args...);
	return ptr;
}
/// @}

} // end namespace anki

