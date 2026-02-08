// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Window/Common.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/CVarSet.h>

namespace anki {

ANKI_CVAR(NumericCVar<U32>, Window, Width, 1920, 16, 16 * 1024, "Width")
ANKI_CVAR(NumericCVar<U32>, Window, Height, 1080, 16, 16 * 1024, "Height")
ANKI_CVAR(NumericCVar<U32>, Window, Fullscreen, 1, 0, 2, "0: windowed, 1: borderless fullscreen, 2: exclusive fullscreen")
ANKI_CVAR(BoolCVar, Window, Maximized, false, "Maximize")
ANKI_CVAR(BoolCVar, Window, Borderless, true, "Borderless")

// Native window.
class NativeWindow : public MakeSingletonPtr<NativeWindow>
{
	template<typename>
	friend class MakeSingletonPtr;

public:
	Error init(U32 targetFps, CString title);

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
NativeWindow& MakeSingletonPtr<NativeWindow>::allocateSingleton<>();

template<>
void MakeSingletonPtr<NativeWindow>::freeSingleton();

} // end namespace anki
