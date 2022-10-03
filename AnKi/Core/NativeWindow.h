// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>

namespace anki {

/// Window initializer
class NativeWindowInitInfo
{
public:
	AllocAlignedCallback m_allocCallback = nullptr;
	void* m_allocCallbackUserData = nullptr;

	U32 m_width = 1920;
	U32 m_height = 1080;
	Array<U32, 4> m_rgbaBits = {8, 8, 8, 0};
	U32 m_depthBits = 0;
	U32 m_stencilBits = 0;
	U32 m_samplesCount = 0;
	static constexpr Bool m_doubleBuffer = true;
	/// Create a fullscreen window with the desktop's resolution
	Bool m_fullscreenDesktopRez = false;
	Bool m_exclusiveFullscreen = false;

	CString m_title = "AnKi";
};

/// Native window.
class NativeWindow
{
public:
	static Error newInstance(const NativeWindowInitInfo& initInfo, NativeWindow*& nativeWindow);

	static void deleteInstance(NativeWindow* nativeWindow);

	U32 getWidth() const
	{
		return m_width;
	}

	U32 getHeight() const
	{
		return m_height;
	}

	F32 getAspectRatio() const
	{
		return F32(m_width) / F32(m_height);
	}

	void setWindowTitle(CString title);

protected:
	U32 m_width = 0;
	U32 m_height = 0;

	HeapMemoryPool m_pool;

	NativeWindow()
	{
	}

	~NativeWindow()
	{
	}
};

} // end namespace anki
