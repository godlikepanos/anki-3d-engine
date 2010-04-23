#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <vector>

using namespace std;


//=====================================================================================================================================
// misc types                                                                                                                         =
//=====================================================================================================================================
#ifndef uchar
typedef unsigned char uchar;
#endif

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef ushort
typedef unsigned short int ushort;
#endif

#ifndef ulong
typedef unsigned long int ulong;
#endif



//=====================================================================================================================================
// MACROS                                                                                                                             =
//=====================================================================================================================================

namespace Util {
extern char*  cutPath( const char* path );
extern string getFunctionFromPrettyFunction( const char* pretty_function );
}

#define __FILENAME__ Util::cutPath( __FILE__ )

#ifdef __GNUG__
	#define __G_FUNCTION__ Util::getFunctionFromPrettyFunction( __PRETTY_FUNCTION__ )
#else
	#define __G_FUNCTION__ __func__
#endif

#if defined( _TERMINAL_COLORING_ ) && defined( _PLATFORM_LINUX_ )
		// for the colors and formating see http://www.dreamincode.net/forums/showtopic75171.htm
    #define COL_ERROR "\033[6;31;6m"
    #define COL_WARNING "\033[6;33;6m"
    #define COL_INFO "\033[6;32;6m"
    #define COL_FATAL "\033[1;31;6m"
    #define COL_DEBUG_ERR "\033[6;31;6m"
    #define COL_DEFAULT "\033[0;;m"
#else
    #define COL_ERROR ""
    #define COL_WARNING ""
    #define COL_INFO ""
    #define COL_FATAL ""
    #define COL_DEBUG_ERR ""
    #define COL_DEFAULT ""
#endif

/// For internal use
#define GENERAL_ERR( x, y, col ) \
	cerr << col << x << " (" << __FILENAME__ << ":" << __LINE__ << " " << __G_FUNCTION__ << "): " << y << COL_DEFAULT << endl;

/// For internal use
#define GENERAL_MSG( x, y, col ) \
	cout << col << x << " (" << __FILENAME__ << ":" << __LINE__ << " " << __G_FUNCTION__ << "): " << y << COL_DEFAULT << endl;

/// Use it like this: ERROR( "tralala" << 10 << ' ' )
#define ERROR( x ) GENERAL_ERR( "Error", x, COL_ERROR )

/// Show a warning
#define WARNING( x ) GENERAL_ERR( "Warning", x, COL_WARNING )

/// Show an error and exit application
#define FATAL( x ) { GENERAL_ERR( "Fatal", x << ". Bye!", COL_FATAL ); exit( EXIT_FAILURE ); }

/// Show an info message
#define INFO( x ) { GENERAL_MSG( "Info", x, COL_INFO ) }

/// Reverse assertion
#ifdef _DEBUG_
	#define DEBUG_ERR( x ) \
		if( x ) \
			GENERAL_ERR( "Debug err", #x, COL_DEBUG_ERR )
#else
    #define DEBUG_ERR( x )
#endif


/// code that executes on debug
#ifdef _DEBUG_
	#define DEBUG_CODE( x ) {x}
#else
	#define DEBUG_CODE( x ) {}
#endif


/// a line so I dont have to write the same crap all the time
#define FOREACH( x ) for( int i=0; i<x; i++ )


/**
 * Read write property
 *
 * - It creates a unique type so it can work with pointers
 * - The get funcs are coming into two flavors, one const and one non-const. The property is read-write after all so the non-const is
 *   acceptable
 * - Dont use it with semicolon at the end (eg PROPERTY_RW( .... );) because of a doxygen bug
 */
#define PROPERTY_RW( __Type__, __varName__, __setFunc__, __getFunc__ ) \
	private: \
		typedef __Type__ __Dummy__##__varName__; \
		__Dummy__##__varName__ __varName__; \
	public: \
		void __setFunc__( const __Dummy__##__varName__& __x__ ) { \
			__varName__ = __x__; \
		} \
		const __Dummy__##__varName__& __getFunc__() const { \
			return __varName__; \
		} \
		__Dummy__##__varName__& __getFunc__() { \
			return __varName__; \
		}

/**
 * Read only property
 *
 * - It creates a unique type so it can work with pointers
 * - Dont use it with semicolon at the end (eg PROPERTY_RW( .... );) because of a doxygen bug
 */
#define PROPERTY_R( __Type__, __varName__, __getFunc__ ) \
	private: \
		typedef __Type__ __Dummy__##__varName__; \
		__Dummy__##__varName__ __varName__; \
	public: \
		const __Dummy__##__varName__& __getFunc__() const { \
			return __varName__; \
		}


/// Just print
#define PRINT( x ) cout << x << endl;


/// BUFFER_OFFSET
#define BUFFER_OFFSET( i ) ((char *)NULL + (i))


//=====================================================================================================================================
// MemZero                                                                                                                            =
//=====================================================================================================================================
/// sets memory to zero
template <typename Type> inline void MemZero( Type& t )
{
	memset( &t, 0, sizeof(Type) );
}


//=====================================================================================================================================
// Vec                                                                                                                                =
//=====================================================================================================================================
/**
 * This is a wrapper of std::vector that adds new functionality
 */
template<typename Type> class Vec: public vector<Type>
{
	public:
		Vec(): vector<Type>() {}
		Vec( size_t size ): vector<Type>(size) {}
		Vec( size_t size, Type val ): vector<Type>(val,size) {}

		Type& operator[]( size_t n )
		{
			DEBUG_ERR( n >= vector<Type>::size() );
			return vector<Type>::operator []( n );
		}

		const Type& operator[]( size_t n ) const
		{
			DEBUG_ERR( n >= vector<Type>::size() );
			return vector<Type>::operator []( n );
		}

		size_t getSizeInBytes() const
		{
			return vector<Type>::size() * sizeof(Type);
		}
};


//=====================================================================================================================================
// Memory allocation information for Linux                                                                                            =
//=====================================================================================================================================
#if defined( _PLATFORM_LINUX_ )

#include <malloc.h>

typedef struct mallinfo Mallinfo; 

inline Mallinfo GetMallInfo()
{
	return mallinfo();
}

inline void printMallInfo( const Mallinfo& minfo )
{
	PRINT( "used:" << minfo.uordblks << " free:" << minfo.fordblks << " total:" << minfo.arena );
}

inline void printMallInfoDiff( const Mallinfo& prev, const Mallinfo& now )
{
	Mallinfo diff;
	diff.uordblks = now.uordblks-prev.uordblks;
	diff.fordblks = now.fordblks-prev.fordblks;
	diff.arena = now.arena-prev.arena;
	printMallInfo( diff );
}

#define MALLINFO_BEGIN Mallinfo __m__ = GetMallInfo();

#define MALLINFO_END printMallInfoDiff( __m__, GetMallInfo() ); 

#endif


//=====================================================================================================================================
// Application                                                                                                                        =
//=====================================================================================================================================
/**
 * The only public variable @see App
 */
extern class App* app;


#endif
