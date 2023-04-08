// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Window/Input.h>
#include <AnKi/Window/KeyCode.h>
#include <android_native_app_glue.h>

namespace anki {

/// Android input implementation
class InputAndroid : public Input
{
public:
	Error initInternal();

	void handleAndroidEvents(android_app* app, int32_t cmd);
	int handleAndroidInput(android_app* app, AInputEvent* event);
};

} // end namespace anki
