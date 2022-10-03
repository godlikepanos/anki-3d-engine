// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Core/NativeWindowAndroid.h>

namespace anki {

Error NativeWindow::newInstance(const NativeWindowInitInfo& initInfo, NativeWindow*& nativeWindow)
{
	NativeWindowAndroid* andwin = static_cast<NativeWindowAndroid*>(initInfo.m_allocCallback(
		initInfo.m_allocCallbackUserData, nullptr, sizeof(NativeWindowAndroid), alignof(NativeWindowAndroid)));
	callConstructor(*andwin);

	const Error err = andwin->init(initInfo);
	if(err)
	{
		callDestructor(*andwin);
		initInfo.m_allocCallback(initInfo.m_allocCallbackUserData, andwin, 0, 0);
		nativeWindow = nullptr;
		return err;
	}
	else
	{
		nativeWindow = andwin;
		return Error::kNone;
	}
}

void NativeWindow::deleteInstance(NativeWindow* window)
{
	if(window)
	{
		NativeWindowAndroid* self = static_cast<NativeWindowAndroid*>(window);
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

NativeWindowAndroid::~NativeWindowAndroid()
{
	ANKI_CORE_LOGI("Destroying Android window");
	ANativeActivity_finish(g_androidApp->activity);

	// Loop until destroyRequested is set
	while(!g_androidApp->destroyRequested)
	{
		int ident;
		int events;
		android_poll_source* source;

		while((ident = ALooper_pollAll(0, nullptr, &events, reinterpret_cast<void**>(&source))) >= 0)
		{
			if(source != nullptr)
			{
				source->process(g_androidApp, source);
			}
		}
	}

	m_nativeWindow = nullptr;
}

Error NativeWindowAndroid::init([[maybe_unused]] const NativeWindowInitInfo& init)
{
	ANKI_CORE_LOGI("Initializing Android window");

	m_pool.init(init.m_allocCallback, init.m_allocCallbackUserData);

	// Loop until the window is ready
	while(g_androidApp->window == nullptr)
	{
		int ident;
		int events;
		android_poll_source* source;

		const int timeoutMs = 5;
		while((ident = ALooper_pollAll(timeoutMs, nullptr, &events, reinterpret_cast<void**>(&source))) >= 0)
		{
			if(source != nullptr)
			{
				source->process(g_androidApp, source);
			}
		}
	}

	m_nativeWindow = g_androidApp->window;

	// Set some stuff
	m_width = ANativeWindow_getWidth(g_androidApp->window);
	m_height = ANativeWindow_getHeight(g_androidApp->window);

	return Error::kNone;
}

} // end namespace anki
