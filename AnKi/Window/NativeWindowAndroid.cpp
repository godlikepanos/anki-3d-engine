// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Window/NativeWindowAndroid.h>

namespace anki {

template<>
template<>
NativeWindow& MakeSingletonPtr<NativeWindow>::allocateSingleton<>()
{
	ANKI_ASSERT(m_global == nullptr);
	m_global = new NativeWindowAndroid();
#if ANKI_ENABLE_ASSERTIONS
	++g_singletonsAllocated;
#endif
	return *m_global;
}

template<>
void MakeSingletonPtr<NativeWindow>::freeSingleton()
{
	if(m_global)
	{
		delete static_cast<NativeWindowAndroid*>(m_global);
		m_global = nullptr;
#if ANKI_ENABLE_ASSERTIONS
		--g_singletonsAllocated;
#endif
	}
}

Error NativeWindow::init(const NativeWindowInitInfo& inf)
{
	return static_cast<NativeWindowAndroid*>(this)->initInternal(inf);
}

void NativeWindow::setWindowTitle([[maybe_unused]] CString title)
{
	// Nothing
}

NativeWindowAndroid::~NativeWindowAndroid()
{
	ANKI_WIND_LOGI("Destroying Android window");
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

	m_nativeWindowAndroid = nullptr;
}

Error NativeWindowAndroid::initInternal([[maybe_unused]] const NativeWindowInitInfo& init)
{
	ANKI_WIND_LOGI("Initializing Android window");

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

	m_nativeWindowAndroid = g_androidApp->window;

	if(init.m_targetFps)
	{
		ANativeWindow_setFrameRate(m_nativeWindowAndroid, init.m_targetFps,
								   ANATIVEWINDOW_FRAME_RATE_COMPATIBILITY_DEFAULT);
	}

	// Set some stuff
	m_width = ANativeWindow_getWidth(g_androidApp->window);
	m_height = ANativeWindow_getHeight(g_androidApp->window);

	return Error::kNone;
}

} // end namespace anki
