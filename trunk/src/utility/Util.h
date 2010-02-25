#ifndef _UTIL_H_
#define _UTIL_H_

#include "Common.h"

namespace Util {

extern int   randRange( int min, int max );
extern float randRange( float min, float max );

extern string      readFile( const char* filename );
extern Vec<string> getFileLines( const char* filename );
extern char*       getFileExtension( const char* path );
extern char*       cutPath( const char* path );
extern string      getPath( const char* path );
extern string      getFunctionFromPrettyFunction( const char* pretty_function );
extern string      intToStr( int );
extern string      floatToStr( float );

}

#endif
