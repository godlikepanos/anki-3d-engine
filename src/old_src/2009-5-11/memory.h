#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <SDL.h>
#include "common.h"

#define MEGABYTE 1048576
#define KILOBYTE 1024

namespace mem {

// for PrintInfo
const uint PRINT_ALL    = 0x0001;
const uint PRINT_HEADER = 0x0002;
const uint PRINT_BLOCKS = 0x0004;

// for Enable
const uint THREADS         = 0x0001;
const uint PRINT_ERRORS    = 0x0002;
const uint PRINT_CALL_INFO = 0x0004;


extern uint malloc_called_num, calloc_called_num, realloc_called_num, free_called_num, new_called_num, delete_called_num;
extern size_t free_size;
extern const size_t buffer_size;
extern uint errors_num;

extern void PrintInfo( uint flags=PRINT_ALL );
extern void Enable( uint flags );
extern void Disable( uint flags );

} // end namespace


//#define _USE_MEM_MANAGER_ // comment this line if you dont want to use mem manager

// MACROS
#ifdef _USE_MEM_MANAGER_

extern void* malloc( size_t size, const char* file, int line, const char* func );
extern void* realloc( void* p, size_t size, const char* file, int line, const char* func );
extern void* calloc( size_t num, size_t size, const char* file, int line, const char* func );
extern void  free( void* p, const char* file, int line, const char* func );
extern void* operator new( size_t size, const char* file, int line, const char* func );
extern void* operator new[]( size_t size, const char* file, int line, const char* func );
extern void  operator delete( void* p, const char* file, int line, const char* func );
extern void  operator delete[]( void* p, const char* file, int line, const char* func );

#define new new( __FILENAME__, __LINE__, __FUNCTION__ )
#define malloc( x ) malloc( x, __FILENAME__, __LINE__, __FUNCTION__ )
#define realloc( x, y ) realloc( x, y, __FILENAME__, __LINE__, __FUNCTION__ )
#define calloc( x, y ) calloc( x, y, __FILENAME__, __LINE__, __FUNCTION__ )
#define free(x) free( x, __FILENAME__, __LINE__, __FUNCTION__ )


#endif


#endif
