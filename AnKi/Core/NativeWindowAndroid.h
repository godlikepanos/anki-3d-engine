// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/NativeWindow.h>
#include <android_native_app_glue.h>

namespace anki {

/// Native window implementation for Android
class NativeWindowAndroid : public NativeWindow
{
public:
	ANativeWindow* m_nativeWindow = nullptr;

	~NativeWindowAndroid();

	Error init(const NativeWindowInitInfo& init);
};

} // end namespace anki
