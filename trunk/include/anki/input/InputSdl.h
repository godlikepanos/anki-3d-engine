#ifndef ANKI_INPUT_INPUT_SDL_H
#define ANKI_INPUT_INPUT_SDL_H

#include "anki/input/KeyCode.h"
#include <SDL_keycode.h>
#include <unordered_map>

namespace anki {

/// SDL input implementation
struct InputImpl
{
	std::unordered_map<SDL_Keycode, KeyCode> sdlKeyToAnki;
};

} // end namespace anki

#endif

