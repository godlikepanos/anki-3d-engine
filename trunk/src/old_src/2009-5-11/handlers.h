#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include <SDL.h>
#include <SDL_syswm.h>
#include "common.h"
#include "input.h"
#include "renderer.h"

namespace hndl {


extern uint timer_tick;

extern void InitWindow( int w, int h, const char* window_caption  );
extern void QuitApp( int code );
extern void WaitForNextFrame();
extern void TogleFullScreen();

inline uint GetTicks()
{
	return SDL_GetTicks();
}


} // end namespace
#endif
