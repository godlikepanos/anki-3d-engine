#include "anki/core/NativeWindowAndroid.h"
#include "anki/core/App.h"
#include "anki/core/Logger.h"
#include "anki/util/Exception.h"
#include "anki/util/Array.h"
#include "anki/util/StdTypes.h"

namespace anki {

//==============================================================================
// NativeWindowImpl                                                            =
//==============================================================================

//==============================================================================
void NativeWindowImpl::create(NativeWindowInitializer& init)
{
	Array<EGLint, 256> attribs;
	U attr = 0;
	EGLint configsCount;
	EGLint format;
	EGLConfig config;

	ANKI_LOGI("Creating native window");

	android_app& andApp = AppSingleton::get().getAndroidApp();

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

	// 
	// EGL config
	//  
	attribs[attr++] = EGL_SURFACE_TYPE;
	attribs[attr++] = EGL_WINDOW_BIT,

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

	if(eglChooseConfig(
		display, &attribs[0], &config, 1, &configsCount) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("Failed to query required EGL configs");
	}

	if(configsCount == 0)
	{
		throw ANKI_EXCEPTION("No matching EGL configs found");
	}

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANKI_ASSERT(andApp.window);
	ANativeWindow_setBuffersGeometry(andApp.window, 0, 0, format);

	// Surface
	//
	surface = eglCreateWindowSurface(display, config, andApp.window, NULL);
	if(surface == EGL_NO_SURFACE)
	{
		throw ANKI_EXCEPTION("Cannot create surface");
	}

	// Context
	//
	EGLint ctxAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};

	context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctxAttribs);
	if(context == EGL_NO_CONTEXT)
	{
		throw ANKI_EXCEPTION("Cannot create context");
	}

	if(eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("Cannot make the context current");
	}

	// Query width and height
	//
	EGLint w, h;
	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	init.width = w;
	init.height = h;
}

//==============================================================================
void NativeWindowImpl::destroy()
{
	if(display != EGL_NO_DISPLAY) 
	{
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

		if(context != EGL_NO_CONTEXT) 
		{
			eglDestroyContext(display, context);
		}

		if(surface != EGL_NO_SURFACE) 
		{
			eglDestroySurface(display, surface);
		}

		eglTerminate(display);
	}
}


//==============================================================================
// NativeWindow                                                                =
//==============================================================================

//==============================================================================
NativeWindow::~NativeWindow()
{}

//==============================================================================
void NativeWindow::create(NativeWindowInitializer& initializer)
{
	impl.reset(new NativeWindowImpl);
	impl->create(initializer);

	// Set the size after because the create may have changed it to something
	// more nice
	width = initializer.width;
	height = initializer.height;
}

//==============================================================================
void NativeWindow::destroy()
{
	impl.reset();
}

//==============================================================================
void NativeWindow::swapBuffers()
{
	ANKI_ASSERT(isCreated());
	if(eglSwapBuffers(impl->display, impl->surface) == EGL_FALSE)
	{
		throw ANKI_EXCEPTION("eglSwapBuffers() failed");
	}
}

} // end namespace anki

