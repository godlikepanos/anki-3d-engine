#include "anki/input/Input.h"
#include <cstring>

namespace anki {

//==============================================================================
void Input::reset(void)
{
	memset(&keys[0], 0, keys.getSize() * sizeof(U32));
	memset(&mouseBtns[0], 0, mouseBtns.getSize() * sizeof(U32));
	mousePosNdc = Vec2(0.0);
}

} // end namespace
