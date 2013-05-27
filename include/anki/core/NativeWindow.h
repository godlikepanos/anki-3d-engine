#ifndef ANKI_CORE_NATIVE_WINDOW_H
#define ANKI_CORE_NATIVE_WINDOW_H

#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include <string>
#include <memory>

namespace anki {

struct NativeWindowImpl;
struct ContextImpl;

/// Window initializer
struct NativeWindowInitializer
{
	Array<U32, 4> rgbaBits = {{8, 8, 8, 8}};
	U32 depthBits = 24;
	U32 stencilBits = 8;
	U32 samplesCount = 0;
	static const Bool doubleBuffer = true;
	Bool fullscreenDesktopRez = false;

	U32 minorVersion = 0;
	U32 majorVersion = 0;
	Bool useGles = false; ///< Use OpenGL ES
	Bool debugContext = false; ///< Enables KHR_debug

	U32 width = 640;
	U32 height = 768;
	std::string title = "Untitled window";
};

/// Native window with GL context
class NativeWindow
{
public:
	NativeWindow()
	{}
	~NativeWindow();

	/// @name Accessors
	/// @{
	NativeWindowImpl& getNative()
	{
		ANKI_ASSERT(isCreated());
		return *impl;
	}

	U32 getWidth() const
	{
		return width;
	}
	U32 getHeight() const
	{
		return height;
	}
	/// @}

	/// @name Public interface
	/// Don't implement them in .h files
	/// @{
	void create(NativeWindowInitializer& initializer);
	void destroy();
	void swapBuffers();
	ContextImpl* createSharedContext();
	void contextMakeCurrent(ContextImpl& ctx);
	/// @}

private:
	U32 width;
	U32 height;

	std::shared_ptr<NativeWindowImpl> impl;

	Bool isCreated() const
	{
		return impl.get() != nullptr;
	}
};

} // end namespace anki

#endif