#ifndef _UI_H_
#define _UI_H_

#include "Common.h"

namespace M {
	class Vec4;
}

namespace Ui { // begin namespace


extern void init(); // exec after init SDL
extern void setColor( const M::Vec4& color );
extern void setPos( float x_, float y_ );
extern void setFontWidth( float w_ );
extern void printf( const char* format, ... );
extern void print( const char* str );


} // end namespace
#endif
