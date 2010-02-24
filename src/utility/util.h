#ifndef _UTIL_H_
#define _UTIL_H_

#include "common.h"

namespace util {

extern int   RandRange( int min, int max );
extern float RandRange( float min, float max );

extern string        ReadFile( const char* filename );
extern Vec<string> GetFileLines( const char* filename );
extern char*         GetFileExtension( const char* path );
extern char*         CutPath( const char* path );
extern string        GetPath( const char* path );
extern string        GetFunctionFromPrettyFunction( const char* pretty_function );

}

#endif
