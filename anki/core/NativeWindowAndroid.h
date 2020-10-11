// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/NativeWindow.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android_native_app_glue.h>

namespace anki
{

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

	void create(NativeWindowInitInfo& init);
	void destroy();
};

} // end namespace anki
