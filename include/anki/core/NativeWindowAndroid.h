// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_NATIVE_WINDOW_ANDROID_H
#define ANKI_CORE_NATIVE_WINDOW_ANDROID_H

#include "anki/core/NativeWindow.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android_native_app_glue.h>

namespace anki {

/// Native window implementation for Android
struct NativeWindowImpl
{
	EGLDisplay display = EGL_NO_DISPLAY;
	EGLSurface surface = EGL_NO_SURFACE;
	EGLContext context = EGL_NO_CONTEXT;

	~NativeWindowImpl()
	{
		destroy();
	}

	void create(NativeWindowInitializer& init);
	void destroy();
};

} // end namespace anki

#endif

