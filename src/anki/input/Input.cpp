// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
	m_mousePosNdc = Vec2(0.0);
	std::memset(&m_events[0], 0, sizeof(m_events));
}

} // end namespace anki
