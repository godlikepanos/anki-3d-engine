#ifndef _APP_H_
#define _APP_H_

#include <SDL/SDL.h>
#include "common.h"

namespace app {


extern uint timer_tick;

extern uint desktop_w;
extern uint desktop_h;

extern uint window_w;
extern uint window_h;

extern void InitWindow();
extern void QuitApp( int code );
extern void WaitForNextFrame();
extern void TogleFullScreen();
extern void printAppInfo();

inline uint GetTicks()
{
	return SDL_GetTicks();
}


} // end namespace
#endif
