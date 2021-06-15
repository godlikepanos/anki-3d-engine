// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Core/Common.h>
#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Allocator.h>

namespace anki
{

class NativeWindowImpl;
using Context = void*;

/// Window initializer
class NativeWindowInitInfo
{
public:
	U32 m_width = 1920;
	U32 m_height = 1080;
	Array<U32, 4> m_rgbaBits = {8, 8, 8, 0};
	U32 m_depthBits = 0;
	U32 m_stencilBits = 0;
	U32 m_samplesCount = 0;
	static const Bool m_doubleBuffer = true;
	/// Create a fullscreen window with the desktop's resolution
	Bool m_fullscreenDesktopRez = false;

	CString m_title = "AnKi";
};

/// Native window.
class NativeWindow
{
public:
	NativeWindow()
	{
	}

	~NativeWindow()
	{
		destroy();
	}

	ANKI_USE_RESULT Error init(NativeWindowInitInfo& initializer, HeapAllocator<U8>& alloc);

	U32 getWidth() const
	{
		return m_width;
	}
	U32 getHeight() const
	{
		return m_height;
	}

	void setWindowTitle(CString title);

	ANKI_INTERNAL HeapAllocator<U8> getAllocator() const
	{
		return m_alloc;
	}

	ANKI_INTERNAL NativeWindowImpl& getNative()
	{
		ANKI_ASSERT(isCreated());
		return *m_impl;
	}

private:
	U32 m_width = 0;
	U32 m_height = 0;

	NativeWindowImpl* m_impl = nullptr;
	HeapAllocator<U8> m_alloc;

	Bool isCreated() const
	{
		return m_impl != nullptr;
	}

	void destroy();
};

} // end namespace anki
