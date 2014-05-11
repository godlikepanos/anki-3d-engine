#include "anki/input/Input.h"
#include <cstring>

namespace anki {

//==============================================================================
void Input::reset(void)
{
	memset(&m_keys[0], 0, m_keys.getSize() * sizeof(U32));
	memset(&m_mouseBtns[0], 0, m_mouseBtns.getSize() * sizeof(U32));
	m_mousePosNdc = Vec2(0.0);
	memset(&m_events[0], 0, sizeof(m_events));
}

} // end namespace anki
