#ifndef ANKI_CORE_NATIVE_WINDOW_GLX_X11_H
#define ANKI_CORE_NATIVE_WINDOW_GLX_X11_H

#include "anki/core/NativeWindow.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glew.h>
#include <GL/glx.h>

namespace anki {

/// Native window implementation for GLX & X11
struct NativeWindowImpl
{
	Display* xDisplay = nullptr;
	Window xWindow = 0;
	Colormap xColormap = 0; ///< Its is actualy an unsigned int
	GLXContext glxContext = nullptr; ///< Its actualy a pointer
	GLXFBConfig* glxConfig = nullptr;

	~NativeWindowImpl()
	{
		destroy();
	}

	void create(NativeWindowInitializer& init);
	void destroy();

private:
	void createInternal(NativeWindowInitializer& init);

	void createDisplay();
	void createConfig(NativeWindowInitializer& init);
	void createNativeWindow(NativeWindowInitializer& init);
	void createContext(NativeWindowInitializer& init);
};

} // end namespace anki

#endif
