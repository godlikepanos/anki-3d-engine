// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/core/NativeWindow.h>
#include <SDL.h>

namespace anki
{

/// Native window implementation for SDL
class NativeWindowImpl
{
public:
	SDL_Window* m_window = nullptr;
};

} // end namespace anki
