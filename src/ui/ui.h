#ifndef _UI_H_
#define _UI_H_

#include "common.h"

namespace M {
	class Vec4;
}

namespace ui { // begin namespace


extern void Init(); // exec after init SDL
extern void SetColor( const M::Vec4& color );
extern void SetPos( float x_, float y_ );
extern void SetFontWidth( float w_ );
extern void printf( const char* format, ... );
extern void print( const char* str );


} // end namespace
#endif
