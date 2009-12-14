#ifndef _INPUT_H_
#define _INPUT_H_

#include <SDL.h>
#include "common.h"
#include "handlers.h"
#include "math.h"

namespace i {


extern void Reset();
extern void HandleEvents();

// keys and btns
extern short keys [SDLK_LAST];  // shows the current key state. 0: unpressed, 1: pressed once, n is >1: kept pressed 'n' times continucely
extern short mouse_btns [8];    // mouse btns. Supporting 3 btns & wheel. Works the same as above

// mouse movement
extern vec2_t mouse_pos_ndc; // the coords are in the NDC space
extern vec2_t mouse_pos;     // the coords are in the window space. (0,0) is in the uper left corner
extern vec2_t mouse_velocity;



} // end namespace
#endif
