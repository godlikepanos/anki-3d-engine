#ifndef ANKI_CORE_NATIVE_WINDOW_EGL_FBDEV_H
#define ANKI_CORE_NATIVE_WINDOW_EGL_FBDEV_H

#include "anki/core/NativeWindow.h"
#define EGL_FBDEV 1
#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace anki {

/// Native window implementation for EGL & FBDEV
struct NativeWindowImpl
{
	EGLDisplay display = EGL_NO_DISPLAY;
	EGLSurface surface = EGL_NO_SURFACE;
	EGLContext context = EGL_NO_CONTEXT;
	EGLNativeWindowType fbwin = 0;

	~NativeWindowImpl()
	{
		destroy();
	}

	void create(NativeWindowInitializer& init);
	void destroy();
};

} // end namespace anki

#endif

