#ifndef _UI_H_
#define _UI_H_

#include "common.h"

namespace m {
	class vec4_t;
}

namespace ui { // begin namespace


extern void Init(); // exec after init SDL
extern void SetColor( const m::vec4_t& color );
extern void SetPos( float x_, float y_ );
extern void SetFontWidth( float w_ );
extern void Printf( const char* format, ... );
extern void Print( const char* str );


} // end namespace
#endif
