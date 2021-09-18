// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Input/Input.h>
#include <AnKi/Core/NativeWindowAndroid.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Core/App.h>
#if ANKI_OS_ANDROID
#	include <android_native_app_glue.h>
#endif

namespace anki
{

static void handleAndroidEvents(android_app* app, int32_t cmd)
{
	Input* input = static_cast<Input*>(app->userData);
	ANKI_ASSERT(input != nullptr);

	switch(cmd)
	{
	case APP_CMD_TERM_WINDOW:
	case APP_CMD_LOST_FOCUS:
		input->addEvent(InputEvent::WINDOW_CLOSED);
		break;
	}
}

Error Input::handleEvents()
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

	return Error::NONE;
}

Error Input::initInternal(NativeWindow*)
{
	g_androidApp->userData = this;
	g_androidApp->onAppCmd = handleAndroidEvents;

	return Error::NONE;
}

void Input::destroy()
{
}

void Input::moveCursor(const Vec2& posNdc)
{
	// do nothing
}

void Input::hideCursor(Bool hide)
{
	// do nothing
}

} // end namespace anki
