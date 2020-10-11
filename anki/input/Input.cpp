// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/input/Input.h>
#include <cstring>

namespace anki
{

void Input::reset()
{
	zeroMemory(m_keys);
	zeroMemory(m_mouseBtns);
	m_mousePosNdc = Vec2(-1.0f);
	m_mousePosWin = UVec2(0u);
	zeroMemory(m_events);
	zeroMemory(m_textInput);
}

} // end namespace anki
