// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Window/Common.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>

namespace anki {

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
	U32 m_targetFps = 0;
	static constexpr Bool m_doubleBuffer = true;
	/// Create a fullscreen window with the desktop's resolution
	Bool m_fullscreenDesktopRez = false;
	Bool m_exclusiveFullscreen = false;

	CString m_title = "AnKi";
};

/// Native window.
class NativeWindow : public MakeSingleton<NativeWindow>
{
	template<typename>
	friend class MakeSingleton;

public:
	Error init(const NativeWindowInitInfo& inf);

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

	NativeWindow()
	{
	}

	~NativeWindow()
	{
	}
};

template<>
template<>
NativeWindow& MakeSingleton<NativeWindow>::allocateSingleton<>();

template<>
void MakeSingleton<NativeWindow>::freeSingleton();

} // end namespace anki
