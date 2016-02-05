// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/util/String.h>

namespace anki
{

// Forward
class ConfigSet;

/// @addtogroup graphics
/// @{

/// A collection of functions that some backends need from other libraries.
class GrManagerInterface
{
public:
	virtual ~GrManagerInterface()
	{
	}

	/// Swap buffers.
	virtual void swapBuffersCommand() = 0;

	/// Make a context current.
	/// @param bind If true make a context current to this thread, if false
	///        make no contexts current to a thread.
	virtual void makeCurrentCommand(Bool bind) = 0;
};

/// Manager initializer.
class GrManagerInitInfo
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;

	WeakPtr<GrManagerInterface> m_interface;

	CString m_cacheDirectory;
	Bool m_registerDebugMessages = false;

	const ConfigSet* m_config = nullptr;
};

/// The graphics manager, owner of all graphics objects.
class GrManager
{
	friend class GrManagerImpl;

public:
	/// Default constructor
	GrManager();

	~GrManager();

	/// Create.
	ANKI_USE_RESULT Error create(GrManagerInitInfo& init);

	/// Swap buffers
	void swapBuffers();

	/// Wait for all work to finish.
	void finish();

	/// Create a new graphics object.
	template<typename T, typename... Args>
	IntrusivePtr<T> newInstance(Args&&... args);

	/// Allocate memory for dynamic buffers. The memory will be reclaimed at
	/// the begining of the next frame.
	void* allocateFrameHostVisibleMemory(
		PtrSize size, BufferUsage usage, DynamicBufferToken& token);

anki_internal:
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

	U64& getUuidIndex()
	{
		return m_uuidIndex;
	}

private:
	GrAllocator<U8> m_alloc; ///< Keep it first to get deleted last
	UniquePtr<GrManagerImpl> m_impl;
	String m_cacheDir;
	U64 m_uuidIndex = 1;
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
