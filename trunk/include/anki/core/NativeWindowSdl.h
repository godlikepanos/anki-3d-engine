// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_NATIVE_WINDOW_SDL_H
#define ANKI_CORE_NATIVE_WINDOW_SDL_H

#include "anki/core/NativeWindow.h"
#include <SDL.h>

namespace anki {

static_assert(sizeof(SDL_GLContext) == sizeof(void*), "Incorrect assumption");

/// Native window implementation for SDL
class NativeWindowImpl
{
public:
	SDL_Window* m_window = nullptr;
	SDL_GLContext m_context = 0;
};

} // end namespace anki

#endif

