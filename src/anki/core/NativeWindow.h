// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/Common.h>
#include <anki/util/StdTypes.h>
#include <anki/util/Array.h>
#include <anki/util/String.h>
#include <anki/util/Allocator.h>

namespace anki
{

class NativeWindowImpl;
using Context = void*;

/// Window initializer
class NativeWindowInitInfo
{
public:
	U32 m_width = 640;
	U32 m_height = 768;
	Array<U32, 4> m_rgbaBits = {8, 8, 8, 0};
	U32 m_depthBits = 0;
	U32 m_stencilBits = 0;
	U32 m_samplesCount = 0;
	static const Bool m_doubleBuffer = true;
	/// Create a fullscreen window with the desktop's resolution
	Bool m_fullscreenDesktopRez = false;

	CString m_title = "Untitled window";
};

/// Native window with GL context
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

	NativeWindowImpl& getNative()
	{
		ANKI_ASSERT(isCreated());
		return *m_impl;
	}

	U32 getWidth() const
	{
		return m_width;
	}
	U32 getHeight() const
	{
		return m_height;
	}

	/// @privatesector
	/// @{
	HeapAllocator<U8>& _getAllocator()
	{
		return m_alloc;
	}
	/// @}

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
