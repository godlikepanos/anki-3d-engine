#ifndef _HUD_H_
#define _HUD_H_

#include "common.h"
#include "math.h"


namespace hud { // begin namespace


extern void Init(); // exec after init SDL
extern void SetColor( const vec4_t& color );
extern void SetPos( float x_, float y_ );
extern void SetFontWidth( float w_ );
extern void Printf( const char* format, ... );
extern void Print( const char* str );


} // end namespace
#endif
