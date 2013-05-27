#include "anki/core/NativeWindowGlxX11.h"
#include "anki/core/Counters.h"
#include "anki/util/Exception.h"
#include <cstring>

namespace anki {

//==============================================================================
// NativeWindowImpl                                                            =
//==============================================================================

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, 
	GLXFBConfig, GLXContext, Bool, const int*);

//==============================================================================
void NativeWindowImpl::createDisplay()
{
	ANKI_ASSERT(xDisplay == nullptr);
	xDisplay = XOpenDisplay(0);
	if(!xDisplay)
	{
		throw ANKI_EXCEPTION("XOpenDisplay() failed");
	}
}

//==============================================================================
void NativeWindowImpl::createConfig(NativeWindowInitializer& init)
{
	Array<int, 256> attribs;
	U at = 0;

	attribs[at++] = GLX_X_RENDERABLE;
	attribs[at++] = True;

	attribs[at++] = GLX_DRAWABLE_TYPE;
	attribs[at++] = GLX_WINDOW_BIT;

	attribs[at++] = GLX_RENDER_TYPE;
	attribs[at++] = GLX_RGBA_BIT;

	attribs[at++] = GLX_X_VISUAL_TYPE;
	attribs[at++] = GLX_TRUE_COLOR;

	attribs[at++] = GLX_RED_SIZE;
	attribs[at++] = init.rgbaBits[0];

	attribs[at++] = GLX_GREEN_SIZE;
	attribs[at++] = init.rgbaBits[1];

	attribs[at++] = GLX_GREEN_SIZE;
	attribs[at++] = init.rgbaBits[2];

	attribs[at++] = GLX_ALPHA_SIZE;
	attribs[at++] = init.rgbaBits[3];

	attribs[at++] = GLX_DEPTH_SIZE;
	attribs[at++] = init.depthBits;

	attribs[at++] = GLX_STENCIL_SIZE;
	attribs[at++] = init.stencilBits;

	attribs[at++] = GLX_DOUBLEBUFFER;
	attribs[at++] = init.doubleBuffer;

	if(init.samplesCount)
	{
		attribs[at++] = GLX_SAMPLE_BUFFERS;
		attribs[at++] = 1;

		attribs[at++] = GLX_SAMPLES;
		attribs[at++] = init.samplesCount;
	}

	attribs[at++] = None;

	int fbcount;
	glxConfig = glXChooseFBConfig(xDisplay, DefaultScreen(xDisplay), 
		&attribs[0], &fbcount);

	if(!glxConfig)
	{
		throw ANKI_EXCEPTION("glXChooseFBConfig() failed");
	}
}

//==============================================================================
void NativeWindowImpl::createNativeWindow(NativeWindowInitializer& init)
{
	XVisualInfo* vi;
	XSetWindowAttributes swa;
	Window root;

	// If it's fullscreen override the resolution with the desktop one
	if(init.fullscreenDesktopRez)
	{
		Screen* pscr = nullptr;
		pscr = DefaultScreenOfDisplay(xDisplay);

		init.width = pscr->width;
		init.height = pscr->height;
	}

	vi = glXGetVisualFromFBConfig(xDisplay, glxConfig[0]);

	if(!vi)
	{
		throw ANKI_EXCEPTION("XGetVisualInfo() failed");
	}

	root = RootWindow(xDisplay, vi->screen);

	/* creating colormap */
	swa.colormap = xColormap = XCreateColormap(xDisplay, root, 
		vi->visual, AllocNone);
	swa.background_pixmap = None;
	swa.border_pixel      = 0;
	swa.event_mask        = StructureNotifyMask;
 
	xWindow = XCreateWindow(
		xDisplay, root, 
		0, 0, init.width, init.height, 0, 
		vi->depth, InputOutput, vi->visual, 
		CWBorderPixel | CWColormap | CWEventMask, 
		&swa);

	if(!xWindow)
	{
		XFree(vi);
		throw ANKI_EXCEPTION("XCreateWindow() failed");
	}

	XStoreName(xDisplay, xWindow, init.title.c_str());
	XMapWindow(xDisplay, xWindow);

	// Toggle fullscreen
	if(init.fullscreenDesktopRez)
	{
		XEvent xev;
		memset(&xev, 0, sizeof(xev));

		Atom wmState = XInternAtom(xDisplay, "_NET_WM_STATE", False);
	    Atom fullscreen = 
			XInternAtom(xDisplay, "_NET_WM_STATE_FULLSCREEN", False);

		xev.type = ClientMessage;
		xev.xclient.window = xWindow;
		xev.xclient.message_type = wmState;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;
		xev.xclient.data.l[1] = fullscreen;
		xev.xclient.data.l[2] = 0;

		XSendEvent(xDisplay, RootWindow(xDisplay, vi->screen), 0,
			SubstructureNotifyMask | SubstructureRedirectMask, &xev);
	}

	// Cleanup
	XFree(vi);
}

//==============================================================================
void NativeWindowImpl::createContext(NativeWindowInitializer& init)
{
	int ctxAttribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, (int)init.majorVersion,
		GLX_CONTEXT_MINOR_VERSION_ARB, (int)init.minorVersion,
		None};

	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;

	glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
		glXGetProcAddressARB((const GLubyte *) "glXCreateContextAttribsARB");

	if(!glXCreateContextAttribsARB)
	{
		throw ANKI_EXCEPTION("Cannot get addres of proc "
			"glXCreateContextAttribsARB");
	}
	
	glxContext = glXCreateContextAttribsARB(xDisplay, *glxConfig, 0,
		True, ctxAttribs);

	XSync(xDisplay, False);

	if(!glxContext)
	{
		throw ANKI_EXCEPTION("glXCreateContextAttribsARB() failed");
	}

	if(!glXMakeCurrent(xDisplay, xWindow, glxContext))
	{
		throw ANKI_EXCEPTION("glXMakeCurrent() failed");
	}
}

//==============================================================================
void NativeWindowImpl::createInternal(NativeWindowInitializer& init)
{
	if(init.useGles)
	{
		throw ANKI_EXCEPTION("GLX and GLES is not supported. "
			"Use other backend");
	}

	// Display
	createDisplay();

	// GLX init
	int major, minor;
	if(!glXQueryVersion(xDisplay, &major, &minor))
	{
		throw ANKI_EXCEPTION("glXQueryVersion() failed");
	}

	if(((major == 1) && (minor < 3)) || (major < 1))
	{
		throw ANKI_EXCEPTION("Old GLX version");
	}

	// Config
	createConfig(init);

	// Window
	createNativeWindow(init);

	// Context
	createContext(init);

	// GLEW
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		throw ANKI_EXCEPTION("GL initialization failed");
	}
	glGetError();
}

//==============================================================================
void NativeWindowImpl::create(NativeWindowInitializer& init)
{
	try
	{
		createInternal(init);
	}
	catch(const std::exception& e)
	{
		destroy();
		throw ANKI_EXCEPTION("Window creation failed") << e;
	}
}

//==============================================================================
void NativeWindowImpl::destroy()
{
	if(glxContext)
	{
		ANKI_ASSERT(xDisplay);
		glXMakeCurrent(xDisplay, 0, 0);
		glXDestroyContext(xDisplay, glxContext);
		glxContext = nullptr;
	}

	if(xWindow)
	{
		ANKI_ASSERT(xDisplay);
		XDestroyWindow(xDisplay, xWindow);
		xWindow = 0;
	}

	if(xColormap)
	{
		ANKI_ASSERT(xDisplay);
		XFreeColormap(xDisplay, xColormap);
	}

	if(xDisplay)
	{
		XCloseDisplay(xDisplay);
		xDisplay = nullptr;
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
	ANKI_COUNTER_START_TIMER(C_SWAP_BUFFERS_TIME);
	ANKI_ASSERT(isCreated());
	glXSwapBuffers(impl->xDisplay, impl->xWindow);
	ANKI_COUNTER_STOP_TIMER_INC(C_SWAP_BUFFERS_TIME);
}

} // end namespace anki