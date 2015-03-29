// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GR_MANAGER_H
#define ANKI_GR_GR_MANAGER_H

#include "anki/gr/Common.h"
#include "anki/util/String.h"

namespace anki {

/// @addtogroup graphics_other
/// @{

/// Manager initializer.
struct GrManagerInitializer
{
	AllocAlignedCallback m_allocCallback; 
	void* m_allocCallbackUserData;

	MakeCurrentCallback m_makeCurrentCallback;
	void* m_makeCurrentCallbackData;
	void* m_ctx;

	SwapBuffersCallback m_swapBuffersCallback;
	void* m_swapBuffersCallbackData;

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
	GrManager() = default;

	~GrManager()
	{
		destroy();
	}

	/// Create.
	ANKI_USE_RESULT Error create(Initializer& init);

	/// Synchronize client and server
	void syncClientServer();

	/// Swap buffers
	void swapBuffers();

	/// Return the alignment of a buffer target
	PtrSize getBufferOffsetAlignment(GLenum target) const;

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
	GrManagerImpl* m_impl = nullptr;
	String m_cacheDir;
	GrAllocator<U8> m_alloc; ///< Keep it last to deleted last

	void destroy();
};
/// @}

} // end namespace anki

#endif
