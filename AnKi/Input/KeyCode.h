// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Enum.h>

namespace anki {

/// Keyboard scancodes taken from SDL
enum class KeyCode
{
	kUnknown = 0,

#define ANKI_KEY_CODE(ak, sdl) k##ak,
#include <AnKi/Input/KeyCode.defs.h>
#undef ANKI_KEY_CODE

	kCount,
	kFirst = 0,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(KeyCode)

enum class MouseButton : U8
{
	kLeft,
	kMiddle,
	kRight,
	kScrollUp,
	kScrollDown,

	kCount
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(MouseButton)

enum class TouchPointer : U8
{
	k0,
	k1,
	k2,
	k3,
	k4,
	k5,
	k6,
	k7,
	k8,
	k9,
	k10,
	k11,
	k12,
	k13,
	k14,
	k15,

	kCount,
	kFirst = k0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(TouchPointer)

} // end namespace anki
