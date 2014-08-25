// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_NATIVE_WINDOW_H
#define ANKI_CORE_NATIVE_WINDOW_H

#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/util/String.h"
#include "anki/util/Allocator.h"

namespace anki {

class NativeWindowImpl;
using Context = void*;

/// Native window with GL context
class NativeWindow
{
public:
	/// Window initializer
	struct Initializer
	{
		U32 m_width = 640;
		U32 m_height = 768;
		Array<U32, 4> m_rgbaBits = {{8, 8, 8, 0}};
		U32 m_depthBits = 0;
		U32 m_stencilBits = 0;
		U32 m_samplesCount = 0;
		static const Bool m_doubleBuffer = true;
		/// Create a fullscreen window with the desktop's resolution
		Bool8 m_fullscreenDesktopRez = false;

		/// @name GL context properties
		/// @{

		/// Minor OpenGL version. Used to create core profile context
		U32 m_minorVersion = 0;
		/// Major OpenGL version. Used to create core profile context
		U32 m_majorVersion = 0;
		Bool8 m_useGles = false; ///< Use OpenGL ES
		Bool8 m_debugContext = false; ///< Enables KHR_debug
		/// @}

		String m_title = "Untitled window";
	};

	NativeWindow(Initializer& initializer, HeapAllocator<U8>& alloc)
	{
		create(initializer, alloc);
	}

	~NativeWindow()
	{
		destroy();
	}

	NativeWindowImpl& getNative()
	{
		ANKI_ASSERT(isCreated());
		return *m_impl;
	}

	U32 getWidth() const
	{
		return m_width;
	}
	U32 getHeight() const
	{
		return m_height;
	}

	void swapBuffers();
	Context createSharedContext();
	Context getCurrentContext();
	void contextMakeCurrent(Context ctx);

	/// @privatesector
	/// @{
	HeapAllocator<U8>& _getAllocator()
	{
		return m_alloc;
	}
	/// @}

private:
	U32 m_width;
	U32 m_height;

	NativeWindowImpl* m_impl;
	HeapAllocator<U8> m_alloc;

	Bool isCreated() const
	{
		return m_impl.get() != nullptr;
	}

	void create(Initializer& initializer, HeapAllocator<U8>& alloc);
	void destroy();
};

} // end namespace anki

#endif
