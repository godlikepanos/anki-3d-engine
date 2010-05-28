#ifndef _UTIL_H_
#define _UTIL_H_

#include "Common.h"

/**
 * The namespace contains a few useful functions
 */
namespace Util {

extern int    randRange( int min, int max ); ///< Pick a random number from min to max
extern uint   randRange( uint min, uint max ); ///< Pick a random number from min to max
extern float  randRange( float min, float max ); ///< Pick a random number from min to max
extern double randRange( double min, double max ); ///< Pick a random number from min to max

extern string      readFile( const char* filename ); ///< Open a text file and return its contents into a string
extern Vec<string> getFileLines( const char* filename ); ///< Open a text file and return its lines into a string vector
extern string      getFileExtension( const char* path ); ///< Self explanatory
extern char*       cutPath( const char* path ); ///< Given a full path return only the file. Used only to cut the path from __FILE__ and return the actual file name and some other cases
extern string      getPath( const char* path ); ///< Get a file and get its path
extern string      getFunctionFromPrettyFunction( const char* pretty_function ); /// The function gets __PRETTY_FUNCTION__ and strips it to get only the function name with its namespace
extern string      intToStr( int ); ///< Self explanatory
extern string      floatToStr( float ); ///< Self explanatory

}

#endif
