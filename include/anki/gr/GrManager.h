// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
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
class NativeWindow;

/// @addtogroup graphics
/// @{

/// Manager initializer.
class GrManagerInitInfo
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;

	CString m_cacheDirectory;

	const ConfigSet* m_config = nullptr;
	NativeWindow* m_window = nullptr;

	/// @name GL context properties
	/// @{

	/// Minor OpenGL version. Used to create core profile context
	U32 m_minorVersion = 0;
	/// Major OpenGL version. Used to create core profile context
	U32 m_majorVersion = 0;
	Bool8 m_useGles = false; ///< Use OpenGL ES
	Bool8 m_debugContext = false; ///< Enables KHR_debug
	/// @}
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
	ANKI_USE_RESULT Error init(GrManagerInitInfo& init);

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
	ptr->init(args...);
	return ptr;
}
/// @}

} // end namespace anki
