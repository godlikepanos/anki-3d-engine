#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL/SDL.h>
#include "Common.h"
#include "App.h"
#include "Math.h"

/// input namespace
namespace I {


extern void reset();
extern void handleEvents();

// keys and btns
extern short keys [];  ///< Shows the current key state. 0: unpressed, 1: pressed once, n is >1: kept pressed 'n' times continucely
extern short mouseBtns [];    ///< Mouse btns. Supporting 3 btns & wheel. Works the same as above

// mouse stuff
extern Vec2 mousePosNdc; ///< The coords are in the NDC space
extern Vec2 mousePos;     ///< The coords are in the window space. (0,0) is in the uper left corner
extern Vec2 mouseVelocity;
extern bool warpMouse;
extern bool hideCursor;



} // end namespace
#endif
