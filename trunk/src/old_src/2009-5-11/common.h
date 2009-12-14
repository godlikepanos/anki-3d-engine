#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <time.h>
#include <fstream>
#include <math.h>
#include <vector>
#include "memory.h"

using namespace std;

/*
=======================================================================================================================================
misc types                                                                                                                            =
=======================================================================================================================================
*/
#ifndef uint
typedef unsigned int uint;
#endif

#ifndef ushort
typedef unsigned short int ushort;
#endif

#ifndef ulong
typedef unsigned long int ulong;
#endif


/*
=======================================================================================================================================
misc funcs                                                                                                                            =
=======================================================================================================================================
*/
extern int   RandRange( int min, int max );
extern float RandRange( float min, float max );
extern char* ReadFile( const char* filename );
extern char* CutPath( const char* path );
extern void  StartBench();
extern clock_t StopBench();


/*
=======================================================================================================================================
MACROS                                                                                                                                =
=======================================================================================================================================
*/

#define __FILENAME__ CutPath( __FILE__ )

// General error macro
#define GENERAL_ERR( x, y ) \
	{cout << "-->" << x << " (" << __FILENAME__ << ":" << __LINE__ << " " << __FUNCTION__ << "): " << y << endl;} \

// ERROR
// in ERROR you can write something like this: ERROR( "tralala" << 10 << ' ' )
#define ERROR( x ) GENERAL_ERR( "ERROR", x )

// WARNING
#define WARNING( x ) GENERAL_ERR( "WARNING", x );

// FATAL ERROR
#define FATAL( x ) { GENERAL_ERR( "FATAL", x << ". Bye!" ); exit( EXIT_FAILURE ); };

// INFO
#define INFO( x ) GENERAL_ERR( "INFO", x );

// DEBUG_ERR
#ifdef _DEBUG
	#define DEBUG_ERR( x ) \
		if( x ) { \
			GENERAL_ERR( "DEBUG_ERR", "See file" ); \
		}
#else
	#define DEBUG_ERR( x )
#endif

// code that executes on debug
#ifdef _DEBUG
	#define DEBUG_CODE( x ) {x}
#else
	#define DEBUG_CODE( x ) {}
#endif


/*
=======================================================================================================================================
MemZero                                                                                                                               =
=======================================================================================================================================
*/
template <typename type_t> inline void MemZero( type_t* t )
{
	memset( t, 0, sizeof(type_t) );
}


#endif
