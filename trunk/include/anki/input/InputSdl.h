// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_INPUT_INPUT_SDL_H
#define ANKI_INPUT_INPUT_SDL_H

#include "anki/input/KeyCode.h"
#include <SDL_keycode.h>
#include <unordered_map>

namespace anki {

/// SDL input implementation
class InputImpl
{
public:
	std::unordered_map<SDL_Keycode, KeyCode> m_sdlToAnki;
};

} // end namespace anki

#endif

