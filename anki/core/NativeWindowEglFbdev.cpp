// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/core/NativeWindowEglFbdev.h>
#include <anki/util/Vector.h>
#include <anki/util/Array.h>
#include <anki/util/StdTypes.h>

namespace anki
{

void NativeWindowImpl::create(NativeWindowInitInfo& init)
{
	// Create window
	//
	fbwin = (fbdev_window*)malloc(sizeof(fbdev_window));
	if(fbwin == NULL)
	{
		throw ANKI_EXCEPTION("malloc() failed");
	}

	fbwin->width = init.width;
	fbwin->height = init.height;

	// EGL init
	//
	display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if(display == EGL_NO_DISPLAY)
	{
		throw ANKI_EXCEPTION("Failed to create display");
	}

	int major, minor;
	if(eglInitialize(display, &major, &minor) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("Failed to initialize EGL");
	}

	int maxConfigs;
	if(eglGetConfigs(display, NULL, 0, &maxConfigs) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("Failed to query EGL configs");
	}

	if(maxConfigs < 1)
	{
		throw ANKI_EXCEPTION("Error in number of configs");
	}

	Vector<EGLConfig> configs(maxConfigs);

	Array<EGLint, 256> attribs;
	U attr = 0;
	attribs[attr++] = EGL_RENDERABLE_TYPE;
	attribs[attr++] = EGL_OPENGL_ES2_BIT;

	if(init.samplesCount > 1)
	{
		attribs[attr++] = EGL_SAMPLES;
		attribs[attr++] = init.samplesCount;
	}

	attribs[attr++] = EGL_RED_SIZE;
	attribs[attr++] = init.rgbaBits[0];
	attribs[attr++] = EGL_GREEN_SIZE;
	attribs[attr++] = init.rgbaBits[1];
	attribs[attr++] = EGL_BLUE_SIZE;
	attribs[attr++] = init.rgbaBits[2];
	attribs[attr++] = EGL_ALPHA_SIZE;
	attribs[attr++] = init.rgbaBits[3];

	attribs[attr++] = EGL_DEPTH_SIZE;
	attribs[attr++] = init.depthBits;
	attribs[attr++] = EGL_STENCIL_SIZE;
	attribs[attr++] = init.stencilBits;

	attribs[attr++] = EGL_NONE;

	EGLint configsCount;
	if(eglChooseConfig(display, &attribs[0], &configs[0], maxConfigs, &configsCount) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("Failed to query required EGL configs");
	}

	if(configsCount == 0)
	{
		throw ANKI_EXCEPTION("No matching EGL configs found");
	}

	EGLConfig config_ = nullptr;
	for(EGLint i = 0; i < configsCount; i++)
	{
		EGLint value;
		EGLConfig config = configs[i];

		// Use this to explicitly check that the EGL config has the
		// expected color depths
		eglGetConfigAttrib(display, config, EGL_RED_SIZE, &value);
		if(value != (EGLint)init.rgbaBits[0])
		{
			continue;
		}

		eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &value);
		if(value != (EGLint)init.rgbaBits[1])
		{
			continue;
		}

		eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &value);
		if(value != (EGLint)init.rgbaBits[2])
		{
			continue;
		}

		eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &value);
		if(value != (EGLint)init.rgbaBits[3])
		{
			continue;
		}

		eglGetConfigAttrib(display, config, EGL_SAMPLES, &value);
		if(value != (EGLint)init.samplesCount)
		{
			continue;
		}

		eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &value);
		if(value != (EGLint)init.depthBits)
		{
			continue;
		}

		eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &value);
		if(value != (EGLint)init.stencilBits)
		{
			continue;
		}

		// We found what we wanted
		config_ = config;
		break;
	}

	if(config_ == nullptr)
	{
		throw ANKI_EXCEPTION("Unable to find suitable EGL config");
	}

	// Surface
	//
	surface = eglCreateWindowSurface(display, config_, static_cast<EGLNativeWindowType>(fbwin), NULL);

	if(surface == EGL_NO_SURFACE)
	{
		throw ANKI_EXCEPTION("Cannot create surface");
	}

	// Context
	//
	EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

	context = eglCreateContext(display, config_, EGL_NO_CONTEXT, ctxAttribs);

	if(context == EGL_NO_CONTEXT)
	{
		throw ANKI_EXCEPTION("Cannot create context");
	}

	if(eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("Cannot make the context current");
	}
}

void NativeWindowImpl::destroy()
{
	// XXX
}

NativeWindow::~NativeWindow()
{
}

void NativeWindow::init(NativeWindowInitInfo& initializer)
{
	impl.reset(new NativeWindowImpl);
	impl->create(initializer);

	// Set the size after because the create may have changed it to something
	// more nice
	width = initializer.width;
	height = initializer.height;
}

void NativeWindow::destroy()
{
	impl.reset();
}

void NativeWindow::swapBuffers()
{
	ANKI_ASSERT(isCreated());
	if(eglSwapBuffers(impl->display, impl->surface) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("eglSwapBuffers() failed");
	}
}

} // end namespace anki
