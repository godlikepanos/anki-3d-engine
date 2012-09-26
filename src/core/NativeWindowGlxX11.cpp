#include "anki/core/NativeWindowGlxX11.h"
#include "anki/util/Exception.h"

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

	vi = glXGetVisualFromFBConfig(xDisplay, glxConfig[0]);

	if(!vi)
	{
		throw ANKI_EXCEPTION("XGetVisualInfo() failed");
	}

	root = RootWindow(xDisplay, vi->screen);

	/* creating colormap */
	swa.colormap = xColormap = XCreateColormap(xDisplay, root, 
		vi->visual, AllocNone);
	swa.background_pixmap = None ;
	swa.border_pixel      = 0;
	swa.event_mask        = StructureNotifyMask;
 
	xWindow = XCreateWindow(
		xDisplay, root, 
		0, 0, init.width, init.height, 0, 
		vi->depth, InputOutput, vi->visual, 
		CWBorderPixel | CWColormap | CWEventMask, 
		&swa);

	XFree(vi);

	if(!xWindow)
	{
		throw ANKI_EXCEPTION("XCreateWindow() failed");
	}

	XStoreName(xDisplay, xWindow, init.title.c_str());
	XMapWindow(xDisplay, xWindow);
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
}

//==============================================================================
void NativeWindow::destroy()
{
	impl.reset(nullptr);
}

} // end namespace anki
