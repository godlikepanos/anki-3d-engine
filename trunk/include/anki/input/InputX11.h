#ifndef ANKI_INPUT_INPUT_X11_H
#define ANKI_INPUT_INPUT_X11_H

#include <X11/XKBlib.h>

namespace anki {

/// X11 input implementation
struct InputImpl
{
	Cursor emptyCursor = 0;
	Array<U16, 256> nativeKeyToAnki;
};

} // end namespace anki

#endif
