// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Input/Input.h>

namespace anki {

Error Input::initInternal(NativeWindow* nativeWindow)
{
	return Error::NONE;
}

void Input::destroy()
{
}

Error Input::handleEvents()
{
	return Error::NONE;
}

void Input::moveCursor(const Vec2& pos)
{
}

void Input::hideCursor(Bool hide)
{
}

} // end namespace anki
