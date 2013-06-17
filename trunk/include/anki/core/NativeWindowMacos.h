#ifndef ANKI_CORE_NATIVE_WINDOW_MACOS_H
#define ANKI_CORE_NATIVE_WINDOW_MACOS_H

#include "anki/core/NativeWindow.h"
// XXX Include native libraries

namespace anki {

/// Native window implementation for Macos
struct NativeWindowImpl
{
	// XXX Add native data

	~NativeWindowImpl()
	{
		destroy();
	}

	void create(NativeWindowInitializer& init);
	void destroy();

private:
	// XXX Add private methods if needed
};

} // end namespace anki

#endif
