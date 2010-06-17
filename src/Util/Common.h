#ifndef _COMMON_H_
#define _COMMON_H_

#include <cstdlib>
#include <cstdio>
#include <memory.h>
#include <string.h>
#include <fstream>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include <vector>

using namespace std;


//======================================================================================================================
// misc types                                                                                                          =
//======================================================================================================================
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

extern ostream& msgPrefix(MsgType msgType, const char* file, int line, const char* func);
extern ostream& msgSuffix(ostream& cs);
extern ostream& msgSuffixFatal(ostream& cs);
extern bool msgGlError(const char* file, int line, const char* func);

#ifdef __GNUG__
	#define FUNCTION __PRETTY_FUNCTION__
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
#if defined(DEBUG_ENABLED)
	#define DEBUG_ERR(x) \
		if(x) \
			msgPrefix(MT_DEBUG_ERR, __FILE__, __LINE__, FUNCTION) << #x << msgSuffix
#else
	#define DEBUG_ERR(x)
#endif


/// code that executes on debug
#ifdef DEBUG_ENABLED
	#define DEBUG_CODE if(true)
#else
	#define DEBUG_CODE if(false)
#endif

/// OpenGL check
#define GL_OK() msgGlError(__FILE__, __LINE__, FUNCTION)


/// a line so I dont have to write the same crap all the time
#define FOREACH(x) for(int i=0; i<x; i++)


/**
 * Read write property
 *
 * - It creates a unique type so it can work with pointers
 * - The get funcs are coming into two flavors, one const and one non-const. The property is read-write after all so the
 *   non-const is acceptable
 * - Dont use it with semicolon at the end (eg PROPERTY_RW(....);) because of a doxygen bug
 */
#define PROPERTY_RW(__Type__, __varName__, __setFunc__, __getFunc__) \
	protected: \
		typedef __Type__ __Dummy__##__varName__; \
		__Dummy__##__varName__ __varName__; \
	public: \
		void __setFunc__(const __Dummy__##__varName__& __x__) { \
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
 * - Dont use it with semicolon at the end (eg PROPERTY_RW(....);) because of a doxygen bug
 */
#define PROPERTY_R(__Type__, __varName__, __getFunc__) \
	protected: \
		typedef __Type__ __Dummy__##__varName__; \
		__Dummy__##__varName__ __varName__; \
	public: \
		const __Dummy__##__varName__& __getFunc__() const { \
			return __varName__; \
		}


/// Just print
#define PRINT(x) cout << x << endl


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
// Vec                                                                                                                 =
//======================================================================================================================
/**
 * This is a wrapper of std::vector that adds new functionality
 */
template<typename Type> class Vec: public vector<Type>
{
	public:
		Vec(): vector<Type>() {}
		Vec(size_t size): vector<Type>(size) {}
		Vec(size_t size, Type val): vector<Type>(size,val) {}

		Type& operator[](size_t n)
		{
			DEBUG_ERR(n >= vector<Type>::size());
			return vector<Type>::operator [](n);
		}

		const Type& operator[](size_t n) const
		{
			DEBUG_ERR(n >= vector<Type>::size());
			return vector<Type>::operator [](n);
		}

		size_t getSizeInBytes() const
		{
			return vector<Type>::size() * sizeof(Type);
		}
};


//======================================================================================================================
// Memory allocation information for Linux                                                                             =
//======================================================================================================================
#if defined(PLATFORM_LINUX)

#include <malloc.h>

typedef struct mallinfo Mallinfo; 

inline Mallinfo GetMallInfo()
{
	return mallinfo();
}

inline void printMallInfo(const Mallinfo& minfo)
{
	PRINT("used:" << minfo.uordblks << " free:" << minfo.fordblks << " total:" << minfo.arena);
}

inline void printMallInfoDiff(const Mallinfo& prev, const Mallinfo& now)
{
	Mallinfo diff;
	diff.uordblks = now.uordblks-prev.uordblks;
	diff.fordblks = now.fordblks-prev.fordblks;
	diff.arena = now.arena-prev.arena;
	printMallInfo(diff);
}

#define MALLINFO_BEGIN Mallinfo __m__ = GetMallInfo();

#define MALLINFO_END printMallInfoDiff(__m__, GetMallInfo()); 

#endif


//======================================================================================================================
// Application                                                                                                         =
//======================================================================================================================
/**
 * The only public variable @see App
 */
extern class App* app;


#endif
