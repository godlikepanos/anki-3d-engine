// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_NATIVE_WINDOW_H
#define ANKI_CORE_NATIVE_WINDOW_H

#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/util/Singleton.h"
#include "anki/util/String.h"
#include <string>
#include <memory>

namespace anki {

class NativeWindowImpl;
typedef void* Context;

/// Window initializer
struct NativeWindowInitializer
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

/// Native window with GL context
class NativeWindow
{
public:
	NativeWindow()
	{}
	~NativeWindow();

	/// @name Accessors
	/// @{
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
	/// @}

	/// @name Public interface
	/// Don't implement them in .h files
	/// @{
	void create(NativeWindowInitializer& initializer);
	void destroy();
	void swapBuffers();
	Context createSharedContext();
	Context getCurrentContext();
	void contextMakeCurrent(Context ctx);
	/// @}

private:
	U32 m_width;
	U32 m_height;

	std::shared_ptr<NativeWindowImpl> m_impl;

	Bool isCreated() const
	{
		return m_impl.get() != nullptr;
	}
};

/// Native window singleton
typedef Singleton<NativeWindow> NativeWindowSingleton;

} // end namespace anki

#endif
