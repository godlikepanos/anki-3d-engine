// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/input/Input.h>
#include <cstring>

namespace anki
{

void Input::reset()
{
	std::memset(&m_keys[0], 0, sizeof(m_keys));
	std::memset(&m_mouseBtns[0], 0, sizeof(m_mouseBtns));
	m_mousePosNdc = Vec2(-1.0f);
	m_mousePosWin = UVec2(0u);
	std::memset(&m_events[0], 0, sizeof(m_events));
}

} // end namespace anki
