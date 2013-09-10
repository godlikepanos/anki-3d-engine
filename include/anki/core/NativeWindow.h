#ifndef ANKI_CORE_NATIVE_WINDOW_H
#define ANKI_CORE_NATIVE_WINDOW_H

#include "anki/util/StdTypes.h"
#include "anki/util/Array.h"
#include "anki/util/Singleton.h"
#include <string>
#include <memory>

namespace anki {

struct NativeWindowImpl;
struct ContextImpl;

/// Window initializer
struct NativeWindowInitializer
{
	U32 width = 640;
	U32 height = 768;
	Array<U32, 4> rgbaBits = {{8, 8, 8, 0}};
	U32 depthBits = 0;
	U32 stencilBits = 0;
	U32 samplesCount = 0;
	static const Bool doubleBuffer = true;
	/// Create a fullscreen window with the desktop's resolution
	Bool fullscreenDesktopRez = false;

	/// @name GL context properties
	/// @{

	/// Minor OpenGL version. Used to create core profile context
	U32 minorVersion = 0;
	/// Major OpenGL version. Used to create core profile context
	U32 majorVersion = 0; 
	Bool useGles = false; ///< Use OpenGL ES
	Bool debugContext = false; ///< Enables KHR_debug
	/// @}

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

/// Native window singleton
typedef Singleton<NativeWindow> NativeWindowSingleton;

} // end namespace anki

#endif
