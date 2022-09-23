// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/NativeWindowHeadless.h>

namespace anki {

Error NativeWindow::newInstance(const NativeWindowInitInfo& initInfo, NativeWindow*& nativeWindow)
{
	HeapAllocator<U8> alloc(initInfo.m_allocCallback, initInfo.m_allocCallbackUserData, "NativeWindow");
	NativeWindowHeadless* hwin = alloc.newInstance<NativeWindowHeadless>();
	hwin->m_alloc = alloc;
	hwin->m_width = initInfo.m_width;
	hwin->m_height = initInfo.m_height;

	nativeWindow = hwin;
	return Error::kNone;
}

void NativeWindow::deleteInstance(NativeWindow* window)
{
	if(window)
	{
		NativeWindowHeadless* self = static_cast<NativeWindowHeadless*>(window);
		HeapAllocator<U8> alloc = self->m_alloc;
		alloc.deleteInstance(self);
	}
}

void NativeWindow::setWindowTitle(CString title)
{
	// Nothing
}

} // end namespace anki
