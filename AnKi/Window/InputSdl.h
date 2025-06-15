// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Window/Input.h>
#include <SDL3/SDL_keycode.h>
#include <unordered_map>

namespace anki {

/// SDL input implementation
class InputSdl : public Input
{
public:
	Error initInternal();
	Error handleEventsInternal();
};

} // end namespace anki
