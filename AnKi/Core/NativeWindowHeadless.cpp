// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/NativeWindowHeadless.h>

namespace anki {

Error NativeWindow::newInstance(const NativeWindowInitInfo& initInfo, NativeWindow*& nativeWindow)
{
	NativeWindowHeadless* hwin = static_cast<NativeWindowHeadless*>(initInfo.m_allocCallback(
		initInfo.m_allocCallbackUserData, nullptr, sizeof(NativeWindowHeadless), alignof(NativeWindowHeadless)));
	callConstructor(*hwin);

	hwin->m_pool.init(initInfo.m_allocCallback, initInfo.m_allocCallbackUserData);
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
		AllocAlignedCallback callback = self->m_pool.getAllocationCallback();
		void* userData = self->m_pool.getAllocationCallbackUserData();
		callDestructor(*self);
		callback(userData, self, 0, 0);
	}
}

void NativeWindow::setWindowTitle([[maybe_unused]] CString title)
{
	// Nothing
}

} // end namespace anki
