// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Window/Input.h>
#include <SDL3/SDL.h>

namespace anki {

/// SDL input implementation
class InputSdl : public Input
{
public:
	Array<SDL_Cursor*, U32(MouseCursor::kCount)> m_cursors = {};

	// SDL calls only from the main thread so have some members for deffer setting some things:
	// {
	MouseCursor m_crntMouseCursor = MouseCursor::kArrow;
	MouseCursor m_prevMouseCursor = MouseCursor::kArrow;

	Bool m_crntHideCursor = false;
	Bool m_prevHideCursor = false;
	// }

	~InputSdl();

	Error initInternal();
	Error handleEventsInternal();
};

} // end namespace anki
