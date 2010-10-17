#ifndef COMMON_H
#define COMMON_H

/*#include <iostream>
#include "StdTypes.h"
#include "Properties.h"
#include "Exception.h"

// dummy define just to use the namespace
namespace boost
{}

namespace M
{}


//======================================================================================================================
// Defines sanity checks                                                                                               =
//======================================================================================================================
#if !defined(DEBUG_ENABLED)
	#error "DEBUG_ENABLED is not defined"
#endif

#if !defined(PLATFORM_LINUX)
	#if !defined(PLATFORM_WIN)
		#error "PLATFORM not defined"
	#endif
#endif


//======================================================================================================================
// MACROS                                                                                                              =
//======================================================================================================================

enum MsgType
{
	MT_ERROR,
	MT_WARNING,
	MT_FATAL,
	MT_DEBUG_ERR,
	MT_INFO,
	MT_NUM
};

extern std::ostream& msgPrefix(MsgType msgType, const char* file, int line, const char* func);
extern std::ostream& msgSuffix(std::ostream& cs);
extern std::ostream& msgSuffixFatal(std::ostream& cs);
extern bool msgGlError(const char* file, int line, const char* func);

#ifdef __GNUG__
	#define FUNCTION __func__
#else
	#define FUNCTION __func__
#endif

/// Use it like this: ERROR("tralala" << 10 << ' ')
#define ERROR(x) msgPrefix(MT_ERROR, __FILE__, __LINE__, FUNCTION) << x << msgSuffix

/// Show a warning
#define WARNING(x) msgPrefix(MT_WARNING, __FILE__, __LINE__, FUNCTION) << x << msgSuffix

/// Show an error and exit application
#define FATAL(x) msgPrefix(MT_FATAL, __FILE__, __LINE__, FUNCTION) << x << ". Bye!" << msgSuffixFatal

/// Show an info message
#define INFO(x) msgPrefix(MT_INFO, __FILE__, __LINE__, FUNCTION) << x << msgSuffix

/// Reverse assertion
#if DEBUG_ENABLED == 1
	#define DEBUG_ERR(x) \
		if(x) \
			msgPrefix(MT_DEBUG_ERR, __FILE__, __LINE__, FUNCTION) << #x << msgSuffix
#else
	#define DEBUG_ERR(x)
#endif


/// code that executes on debug
#if DEBUG_ENABLED == 1
	#define DEBUG_CODE if(true)
#else
	#define DEBUG_CODE if(false)
#endif

/// OpenGL check
#define GL_OK() msgGlError(__FILE__, __LINE__, FUNCTION)


/// a line so I dont have to write the same crap all the time
#define FOREACH(x) for(int i=0; i<x; i++)


/// Just print
#define PRINT(x) std::cout << x << std::endl


/// BUFFER_OFFSET
#define BUFFER_OFFSET(i) ((char *)NULL + (i))


//======================================================================================================================
// memZero                                                                                                             =
//======================================================================================================================
/// sets memory to zero
template <typename Type> inline void memZero(Type& t)
{
	memset(&t, 0, sizeof(Type));
}


//======================================================================================================================
// Application                                                                                                         =
//======================================================================================================================
*
 * The only public variable @see App

extern class App* app;*/


#endif
