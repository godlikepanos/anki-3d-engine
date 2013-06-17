#include "anki/input/Input.h"
#include "anki/input/InputMacos.h"

namespace anki {

//==============================================================================
Input::~Input()
{}

//==============================================================================
void Input::init(NativeWindow* nativeWindow)
{
	// XXX Add initialization code
}

//==============================================================================
void Input::handleEvents()
{
	// XXX Add platform code
}

//==============================================================================
void Input::moveMouse(const Vec2& pos)
{
	// XXX Add platform code
}

//==============================================================================
void Input::hideCursor(Bool hide)
{
	// XXX Add platform code
}

} // end namespace anki
