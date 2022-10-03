// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Input.h>
#include <AnKi/Input/KeyCode.h>
#include <SDL_keycode.h>
#include <unordered_map>

namespace anki {

/// SDL input implementation
class InputSdl : public Input
{
public:
	Error init();
	Error handleEventsInternal();
};

} // end namespace anki
