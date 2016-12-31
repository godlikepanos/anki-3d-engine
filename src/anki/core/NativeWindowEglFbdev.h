// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/NativeWindow.h>
#define EGL_FBDEV 1
#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace anki
{

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

	void create(NativeWindowInitInfo& init);
	void destroy();
};

} // end namespace anki
