#include "anki/core/NativeWindowMacos.h"
#include "anki/core/Counters.h"
#include "anki/util/Exception.h"

namespace anki {

//==============================================================================
// NativeWindowImpl                                                            =
//==============================================================================

//==============================================================================
void NativeWindowImpl::create(NativeWindowInitializer& init)
{
	// XXX Populate
}

//==============================================================================
void NativeWindowImpl::destroy()
{
	// XXX Populate
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
	// XXX Add func to swap buffers
	ANKI_COUNTER_STOP_TIMER_INC(C_SWAP_BUFFERS_TIME);
}

} // end namespace anki
