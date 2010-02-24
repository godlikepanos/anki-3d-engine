#include "memory.h"
#include <SDL/SDL.h>

#ifdef _USE_MEM_MANAGER_

namespace mem {

#ifdef malloc
#undef malloc
#endif

#ifdef realloc
#undef realloc
#endif

#ifdef calloc
#undef calloc
#endif

#ifdef free
#undef free
#endif

#ifdef new
#undef new
#endif

#ifdef delete
#undef delete
#endif


#ifdef _DEBUG_
	#define SANITY_CHECKS SanityChecks();
	#define PRINT_CALL_INFO(x) { if(mem::print_call_info) INFO(x) }
#else
	#define SANITY_CHECKS
	#define PRINT_CALL_INFO(x)
#endif

#define MERROR(x) {++errors_num; if(print_errors) ERROR(x)}

/*
=======================================================================================================================================
variables and types                                                                                                                   =
=======================================================================================================================================
*/

// owner info for the block
struct mblock_owner_t
{
	const char* file;
	int         line;
	const char* func;
};

// used as a list node
struct mem_block_t
{
	void*          addr;
	size_t         size;     // aka offset
	bool           free_space;
	bool           active;   // if true then the block/node is an active node of the list
	uint   id;      // the id in the mem_blocks array
	mem_block_t*   prev;
	mem_block_t*   next;
	mblock_owner_t owner;  // for leak tracking
};

// the buffer
const size_t buffer_size = 30*MEGABYTE;
char prealloced_buff [buffer_size];
void* buffer = prealloced_buff;
size_t free_size = buffer_size;

// block stuff
const int MAX_MEM_BLOCKS = 20*KILOBYTE;
mem_block_t mem_blocks[ MAX_MEM_BLOCKS ]; // this is actualy a list
uint active_mem_blocks_num = 3;
mem_block_t& head_node = mem_blocks[0];
mem_block_t& tail_node = mem_blocks[1];

// dummy
static void DummyFunc() {}

// Used so we can save a check in NewBlock
static void init();
void (*p_Init)(void) = init;

// threads
void (*p_Lock)(void) = DummyFunc;
void (*p_Unlock)(void) = DummyFunc;
SDL_sem* semaphore = NULL;

// unknown owner
mblock_owner_t unknown_owner = {"??", 0, "??"};

// times we called each
uint malloc_called_num = 0;
uint calloc_called_num = 0;
uint realloc_called_num = 0;
uint free_called_num = 0;
uint new_called_num = 0;
uint delete_called_num = 0;

// errors & other
bool print_errors = false;
uint errors_num = 0;
bool print_call_info = false; // works only in debug

/*
=======================================================================================================================================
FreeBlocksNum                                                                                                                         =
=======================================================================================================================================
*/
static int FreeBlocksNum()
{
	mem_block_t* mb = head_node.next;
	int num = 0;
	do
	{
		if( mb->free_space )
			++num;
		mb = mb->next;
	} while( mb != &tail_node );
	return num;
}

/*
=======================================================================================================================================
SetOwner                                                                                                                              =
set the file,func,line to the given block                                                                                             =
=======================================================================================================================================
*/
static inline void SetOwner( mem_block_t* mb, mblock_owner_t* owner )
{
	DEBUG_ERR( mb == &head_node || mb == &tail_node ); // shouldn't change the head_node or tail node
	mb->owner.file = owner->file;
	mb->owner.line = owner->line;
	mb->owner.func = owner->func;
}


/*
=======================================================================================================================================
SanityChecks                                                                                                                          =
=======================================================================================================================================
*/
static bool SanityChecks()
{
	// the head_node
	if( !(head_node.addr == NULL || head_node.size == 0 || head_node.prev == NULL || head_node.id == 0 ||
	    head_node.active == true || head_node.free_space == false) )
		MERROR( "In head_node" );

	// check the list
	uint num = 0;
	for( int i=0; i<MAX_MEM_BLOCKS; i++ )
		if( mem_blocks[i].active ) ++num;

	if( active_mem_blocks_num != num ) MERROR( "In mem_blocks list" );

	// check the size
	size_t size = 0;
	mem_block_t* mb = head_node.next;
	do
	{
		if( !mb->free_space )
			size += mb->size;

		// the prev's next has to be ME and the next's prev has to show me also
		if( mb->prev->next!=mb || mb->next->prev!=mb )
			MERROR( "Chain is broken" );

		if( mb->next!=&tail_node && ((char*)mb->addr)+mb->size!=mb->next->addr )
			MERROR( "In crnt and next sizes cohisency" );

		if( mb->next == NULL || mb->prev==NULL )
			MERROR( "Prev or next are NULL" );

		mb = mb->next;
	} while( mb!=&tail_node );

	if( size != buffer_size-free_size ) MERROR( "In size" );

	return true;
}


/*
=======================================================================================================================================
BytesStr                                                                                                                              =
=======================================================================================================================================
*/
static char* BytesStr( size_t size )
{
	static char str[10];

	if( size > MEGABYTE )
		sprintf( str, "%dMB", (uint)(size/MEGABYTE) );
	else if( size > KILOBYTE )
		sprintf( str, "%dKB", (uint)(size/KILOBYTE) );
	else
		sprintf( str, "%dB ", (uint)(size) );
	return str;
}


/*
=======================================================================================================================================
printBlockInfo                                                                                                                        =
=======================================================================================================================================
*/
static void printBlockInfo( const mem_block_t* mb )
{
	const char cond = (mb->free_space) ? 'F' : 'U';
	cout << setw(4) << setfill(' ') << mb->id << setw(0) << ' ' << cond << ' ' << setw(6) <<  BytesStr( mb->size ) << setw(0) << hex <<
	     " 0x" << mb->addr << dec;

	if( cond=='U' ) cout << " " << mb->owner.file << ' ' << mb->owner.line << ' ' << mb->owner.func;

	cout << endl;
}


/*
=======================================================================================================================================
printInfo                                                                                                                             =
=======================================================================================================================================
*/
void printInfo( uint flags )
{
	cout << "\n=========================== MEM REPORT =========================" << endl;

	// header
	if( (flags & PRINT_ALL)==PRINT_ALL || (flags & PRINT_HEADER)==PRINT_HEADER )
	{
		cout << "Used space: " << BytesStr(buffer_size-free_size) << "(" << buffer_size-free_size << ")";
		cout << ", free: " << BytesStr(free_size) << " (" << free_size << ")";
		cout << ", total: " << BytesStr(buffer_size) << " (" << buffer_size << ")" << endl;

		int num = FreeBlocksNum();
		cout << "Active blocks: " << active_mem_blocks_num << "(free space: " << num << ", used space: " << active_mem_blocks_num-num <<
						"), total: " << MAX_MEM_BLOCKS << endl;

		// get the block with the max free space
		mem_block_t* tmp = head_node.next;
		mem_block_t* mb = &head_node;
		do
		{
			if( tmp->free_space && tmp->size > mb->size )
				mb = tmp;
			tmp = tmp->next;
		} while( tmp!=&tail_node );

		cout << "Block with max free space: " << mb->id << ", size: " << BytesStr( mb->size ) << " (" << mb->size << ")" << endl;

		// print how many times malloc,realloc etc have been called
		cout << "Func calls: malloc:" << malloc_called_num << ", calloc:" << calloc_called_num << ", realloc:" << realloc_called_num <<
		     ", free:" << free_called_num << ", new:" << new_called_num << ", delete:" << delete_called_num << endl;

		cout << "Errors count:" << errors_num << endl;
	}

	// blocks
	if( (flags & PRINT_ALL)==PRINT_ALL || (flags & PRINT_BLOCKS)==PRINT_BLOCKS )
	{
		cout << "Block table (id, type, size [, file, line, func]):" << endl;

		mem_block_t* mb = head_node.next;
		do
		{
			printBlockInfo( mb );
			mb = mb->next;
		} while( mb!=&tail_node );
	}

	cout << "================================================================\n" << endl;
}


/*
=======================================================================================================================================
init                                                                                                                                  =
=======================================================================================================================================
*/
static void init()
{
#ifdef _DEBUG_
	memset( buffer, (char)0xCC, buffer_size );
#endif

	// mem block stuff

	// set the head block. Its the head of the list
	head_node.addr = NULL;
	head_node.size = 0;
	head_node.prev = NULL;
	head_node.next = &mem_blocks[2];
	head_node.id = 0;
	head_node.active = true;
	head_node.free_space = false;

	// set the head block. Its the head of the list
	tail_node.addr = NULL;
	tail_node.size = 0;
	tail_node.prev = &mem_blocks[2];
	tail_node.next = NULL;
	tail_node.id = 1;
	tail_node.active = true;
	tail_node.free_space = false;

	// set the first block
	mem_blocks[2].addr = buffer;
	mem_blocks[2].size = buffer_size;
	mem_blocks[2].prev = &head_node;
	mem_blocks[2].next = &tail_node;
	mem_blocks[2].id = 2;
	mem_blocks[2].active = true;
	mem_blocks[2].free_space = true;

	// set the rest
	memset( &mem_blocks[3], 0, sizeof(mem_block_t)*(MAX_MEM_BLOCKS-3) );
	for( int i=3; i<MAX_MEM_BLOCKS; i++ )
	{
		mem_blocks[i].id = i;
	}

	p_Init = DummyFunc;

	semaphore = SDL_CreateSemaphore(1);
}


/*
=======================================================================================================================================
thread stuff                                                                                                                          =
=======================================================================================================================================
*/
static void Lock()
{
	if( SDL_SemWait(semaphore)==-1 )
		MERROR( "Cant lock semaphore" );
}

static void Unlock()
{
	if( SDL_SemPost(semaphore)==-1 )
		MERROR( "Cant unlock semaphore" );
}


/*
=======================================================================================================================================
Enable                                                                                                                                =
=======================================================================================================================================
*/
void Enable( uint flags )
{
	if( (flags & THREADS)==THREADS )
	{
		p_Lock = Lock;
		p_Unlock = Unlock;
	}

	if( (flags & PRINT_ERRORS)==PRINT_ERRORS )
		print_errors = true;


	if( (flags & PRINT_CALL_INFO)==PRINT_CALL_INFO )
		print_call_info = true;
}


/*
=======================================================================================================================================
Disable                                                                                                                               =
=======================================================================================================================================
*/
void Disable( uint flags )
{
	if( (flags & THREADS)==THREADS )
	{
		p_Lock = DummyFunc;
		p_Unlock = DummyFunc;
	}

	if( (flags & PRINT_ERRORS)==PRINT_ERRORS )
		print_errors = false;

	if( (flags & PRINT_CALL_INFO)==PRINT_CALL_INFO )
		print_call_info = false;
}


/*
=======================================================================================================================================
GetBlock                                                                                                                              =
find the active block who has for addr the given ptr param. Func used by free and realloc                                             =
=======================================================================================================================================
*/
static mem_block_t* GetBlock( void* ptr )
{
	//if( ptr<buffer || ptr>((char*)buffer+buffer_size) ) return &head_node;

	mem_block_t* mb = tail_node.prev;
	do
	{
		if( mb->addr==ptr )
			return mb;
		mb = mb->prev;
	} while( mb!=&head_node );

	return NULL;

//
//	int a = 1;
//	int b = active_mem_blocks_num-2;
//	mem_block_t* mb = head_node.next;
//	int pos = 1;
//
//	for(;;)
//	{
//		int tmp = (a+b)/2;
//
//		// move the mb to crnt_pos
//		if( pos < tmp )
//			for( int i=0; i<tmp-pos; i++ )
//				mb = mb->next;
//		else
//			for( int i=0; i<pos-tmp; i++ )
//				mb = mb->prev;
//		pos = tmp;
//
//		if( ptr < mb->addr )
//			b = pos;
//		else if( ptr > mb->addr )
//			a = pos;
//		else
//			return mb;
//		if( b-a < 2 ) break;
//	}
//
//	return NULL;
}


/*
=======================================================================================================================================
GetInactiveBlock                                                                                                                      =
get an inactive node/block                                                                                                            =
=======================================================================================================================================
*/
static mem_block_t* GetInactiveBlock()
{
	for( int i=2; i<MAX_MEM_BLOCKS; i++ )
	{
		if( !mem_blocks[i].active )
			return &mem_blocks[i];
	}

	FATAL( "Cannot find an inactive node. Inc the mem_blocks arr" );
	return NULL;
}


/*
=======================================================================================================================================
WorstFit                                                                                                                              =
"worst fit" algorithm. It returns the block with the biger free space                                                                 =
=======================================================================================================================================
*/
static mem_block_t* WorstFit( size_t size )
{
	mem_block_t* tmp = tail_node.prev;
	mem_block_t* candidate = &head_node;
	do
	{
		if( tmp->size > candidate->size && tmp->free_space )
			candidate = tmp;
		tmp = tmp->prev;
	} while( tmp!=&head_node );

	return candidate;
}


/*
=======================================================================================================================================
BestFit                                                                                                                               =
=======================================================================================================================================
*/
static mem_block_t* BestFit( size_t size )
{
	mem_block_t* tmp = tail_node.prev;
	mem_block_t* candidate = &head_node;

	// find a free block firstly
	do
	{
		if( tmp->free_space )
		{
			candidate = tmp;
			break;
		}
		tmp = tmp->prev;
	} while( tmp!=&head_node );

	if( candidate == &head_node ) return candidate; // we failed to find free node

	// now run the real deal
	do
	{
		if( tmp->free_space )
		{
			if( (tmp->size < candidate->size) && (tmp->size > size) )
				candidate = tmp;
			else if( tmp->size == size )
				return tmp;
		}
		tmp = tmp->prev;
	} while( tmp!=&head_node );

	return candidate;
}


/*
=======================================================================================================================================
BadFit                                                                                                                                =
=======================================================================================================================================
*/
static mem_block_t* BadFit( size_t size )
{
	mem_block_t* tmp = tail_node.prev;
	do
	{
		if( tmp->size >= size && tmp->free_space )
			return tmp;
		tmp = tmp->prev;
	} while( tmp!=&head_node );

	return &head_node;
}


/*
=======================================================================================================================================
NewBlock                                                                                                                              =
just free the given block                                                                                                             =
=======================================================================================================================================
*/
static mem_block_t* NewBlock( size_t size )
{
	p_Init();

	// a simple check
	if( size < 1 )
	{
		MERROR( "Size is < 1" );
		return &head_node;
	}

	// get an inactive block
	mem_block_t* newmb = GetInactiveBlock();

	// use an algorithm to find the best candidate
	mem_block_t* candidate = BestFit(size);
	if( candidate==&head_node )
	{
		FATAL( "There are no free blocks" );
		return &head_node;
	}

	// case 0: we have found a big enought free block
	if( candidate->size > size )
	{
		// reorganize the prev and the next of the 3 involved blocks
		DEBUG_ERR( candidate->prev==NULL );
		candidate->prev->next = newmb;
		newmb->prev = candidate->prev;
		newmb->next = candidate;
		candidate->prev = newmb;

		// do the rest of the changes
		newmb->addr = candidate->addr;
		newmb->size = size;

		candidate->addr = ((char*)candidate->addr) + size;
		candidate->size -= size;

		newmb->active = true;
		newmb->free_space = false;
		++active_mem_blocks_num;
	}
	// case 1: we have found a block with the exchact space
	else if( candidate->size == size )
	{
		newmb = candidate;
		newmb->free_space = false;
	}
	// case 2: we cannot find a block!!!
	else // if( max_free_bytes < bytes )
	{
		FATAL( "Cant find block with " << size << " free space. Inc buffer" );
		return &head_node;
	}

	free_size -= size;
	return newmb;
}


/*
=======================================================================================================================================
FreeBlock                                                                                                                             =
=======================================================================================================================================
*/
static void FreeBlock( mem_block_t* crnt )
{
	DEBUG_ERR( crnt->free_space || !crnt->active || crnt==&head_node || crnt==&tail_node ); // self explanatory

	free_size += crnt->size;

#ifdef _DEBUG_
	memset( crnt->addr, (char)0xCC, crnt->size );
#endif

	// rearange the blocks
	mem_block_t* prev = crnt->prev;
	mem_block_t* next = crnt->next;
	// if we have a prev block with free space we resize the prev and then we remove the current one
	if( prev != &head_node && prev->free_space )
	{
		prev->size += crnt->size;
		prev->next = next;
		next->prev = prev;

		// remove the crnt block from the list
		crnt->active = false;
		--active_mem_blocks_num;

		// rearange the blocks for the next check
		crnt = prev;
		prev = crnt->prev;
	}

	// if we have a next block with free space we resize the next and then we remove the crnt one
	if( next != &tail_node && next->free_space )
	{
		next->addr = crnt->addr;
		next->size += crnt->size;
		next->prev = prev;
		prev->next = next;

		// remove the next block from the list
		crnt->active = false;
		--active_mem_blocks_num;
	}

	crnt->free_space = true;
}


/*
=======================================================================================================================================
ReallocBlock                                                                                                                          =
it gets the block we want to realloc and returns the reallocated (either the same or a new)                                           =
=======================================================================================================================================
*/
static mem_block_t* ReallocBlock( mem_block_t* crnt, size_t size )
{
	DEBUG_ERR( crnt->free_space || !crnt->active || crnt==&head_node || crnt==&tail_node ); // self explanatory


	// case 0: If size is 0 and p points to an existing block of memory, the memory block pointed by ptr is deallocated and a NULL...
	// ...pointer is returned.(ISO behaviour)
	if( size==0 )
	{
		FreeBlock( crnt );
		crnt = &head_node;
	}
	// case 1: we want more space
	else if( size > crnt->size )
	{
		mem_block_t* next = crnt->next;
		// case 1.0: the next block has enough space. Then we eat from the next
		if( next!=&tail_node && next->free_space && next->size >= size )
		{
			free_size -= size - crnt->size;
			next->addr = ((char*)next->addr) + (size - crnt->size); // shift right the addr
			next->size -= size - crnt->size;
			crnt->size = size;
		}
		// case 1.1: We cannot eat from the next. Create new block and move the crnt's data there
		else
		{
			mem_block_t* mb = NewBlock( size );
			memcpy( mb->addr, crnt->addr, crnt->size );
			FreeBlock( crnt );
			crnt = mb;
		}
	}
	// case 2: we want less space
	else if( size < crnt->size )
	{
		mem_block_t* next = crnt->next;
		// case 2.0: we have next
		if( next!=&tail_node )
		{
			// case 2.0.0: the next block is free space...
			// ...resize next and crnt
			if( next->free_space )
			{
				free_size -= size - crnt->size;
				next->addr = ((char*)next->addr) - (crnt->size - size); // shl
				next->size += crnt->size - size;
				crnt->size = size;
			}
			// case 2.0.1: the next block is used space. Create new free block
			else
			{
				free_size -= size - crnt->size;
				mem_block_t* newmb = GetInactiveBlock();
				newmb->active = true;
				newmb->free_space = true;
				newmb->prev = crnt;
				newmb->next = next;
				newmb->addr = ((char*)crnt->addr) + size;
				newmb->size = crnt->size - size;

				next->prev = newmb;

				crnt->size = size;
				crnt->next = newmb;
			}
		}
		// case 2.1: We DONT have next. Create a new node
		else
		{
			free_size -= size - crnt->size;
			mem_block_t* newmb = GetInactiveBlock();
			newmb->active = true;
			newmb->free_space = true;
			newmb->prev = crnt;
			newmb->next = next;
			newmb->addr = ((char*)crnt->addr) + size;
			newmb->size = crnt->size - size;

			crnt->size = size;
			crnt->next = newmb;
		}
	}

	return crnt;
}


/*
=======================================================================================================================================
Malloc                                                                                                                                =
=======================================================================================================================================
*/
static void* Malloc( size_t size, mblock_owner_t* owner=&unknown_owner )
{
	p_Lock();

	PRINT_CALL_INFO( "caller: \"" << owner->file << ':' << owner->line << "\", size: " << size );

	mem_block_t* mb = NewBlock( size );
	SetOwner( mb, owner );
	SANITY_CHECKS

	p_Unlock();
	return mb->addr;
}


/*
=======================================================================================================================================
Calloc                                                                                                                                =
=======================================================================================================================================
*/
static void* Calloc( size_t num, size_t size, mblock_owner_t* owner=&unknown_owner )
{
	p_Lock();


	PRINT_CALL_INFO( "caller: \"" << owner->file << ':' << owner->line << "size: " << size );

	mem_block_t* mb = NewBlock( num*size );
	SetOwner( mb, owner);
	memset( mb->addr, 0x00000000, num*size );
	SANITY_CHECKS

	p_Unlock();
	return mb->addr;
}


/*
=======================================================================================================================================
Realloc                                                                                                                               =
=======================================================================================================================================
*/
static void* Realloc( void* ptr, size_t size, mblock_owner_t* owner=&unknown_owner )
{
	p_Lock();

	// ISO beheviur
	if( ptr==NULL )
	{
		p_Unlock();
		return Malloc( size, owner );
	}

	// find the block we want to realloc
	mem_block_t* mb = GetBlock( ptr );

	PRINT_CALL_INFO( "caller: \"" << owner->file << ':' << owner->line << "\", user: \"" << mb->owner.file << ':' << mb->owner.line <<
		"\", new size: " << size );

	if( mb==NULL )
	{
		MERROR( "Addr 0x" << hex << ptr << dec << " not found" );
		p_Unlock();
		return NULL;
	}
	if( mb->free_space  )
	{
		MERROR( "Addr 0x" << hex << ptr << dec << " is free space" );
		p_Unlock();
		return NULL;
	}

	mem_block_t* crnt = ReallocBlock( mb, size );
	SetOwner( crnt, owner );
	SANITY_CHECKS

	p_Unlock();
	return crnt->addr;
}


/*
=======================================================================================================================================
Free                                                                                                                                  =
=======================================================================================================================================
*/
static void Free( void* ptr, mblock_owner_t* owner=&unknown_owner )
{
	p_Lock();

	// find the block we want to delete
	mem_block_t* mb = GetBlock( ptr );
	if( mb==NULL )
	{
		MERROR( "Addr 0x" << hex << ptr << dec << " not found" );
		p_Unlock();
		return;
	}
	if( mb->free_space  )
	{
		MERROR( "Addr 0x" << hex << ptr << dec << " is free space" );
		p_Unlock();
		return;
	}

	PRINT_CALL_INFO( "caller: \"" << owner->file << ':' << owner->line << "\", user: \"" << mb->owner.file << ':' << mb->owner.line
		<< "\", mb size: " << mb->size );

	FreeBlock( mb );
	SANITY_CHECKS

	p_Unlock();
}

} // end namespace

/**
=======================================================================================================================================
overloaded stuff                                                                                                                      =
=======================================================================================================================================
*/

// malloc
void* malloc( size_t size ) throw()
{
	++mem::malloc_called_num;
	return mem::Malloc( size );
}

// realloc
void* realloc( void* p, size_t size ) throw()
{
	++mem::realloc_called_num;
	return mem::Realloc( p, size );
}

// calloc
void* calloc( size_t num, size_t size ) throw()
{
	++mem::calloc_called_num;
	return mem::Calloc( num, size );
}

// free
void free( void* p ) throw()
{
	++mem::free_called_num;
	mem::Free( p );
}

// new
void* operator new( size_t size ) throw(std::bad_alloc)
{
	++mem::new_called_num;
	return mem::Malloc( size );
}

// new[]
void* operator new[]( size_t size ) throw(std::bad_alloc)
{
	++mem::new_called_num;
	return mem::Malloc( size );
}

// delete
void operator delete( void* p ) throw()
{
	++mem::delete_called_num;
	mem::Free(p);
}

// delete []
void operator delete[]( void* p ) throw()
{
	++mem::delete_called_num;
	mem::Free(p);
}


/**
=======================================================================================================================================
overloaded stuff with owner                                                                                                           =
=======================================================================================================================================
*/

// malloc
void* malloc( size_t size, const char* file, int line, const char* func )
{
	++mem::malloc_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	return mem::Malloc( size, &owner );
}

// realloc
void* realloc( void* p, size_t size, const char* file, int line, const char* func )
{
	++mem::realloc_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	return mem::Realloc( p, size, &owner );
}

// calloc
void* calloc( size_t num, size_t size, const char* file, int line, const char* func )
{
	++mem::calloc_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	return mem::Calloc( num, size, &owner );
}

// free
void free( void* p, const char* file, int line, const char* func )
{
	++mem::free_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	mem::Free( p, &owner );
}

// new
void* operator new( size_t size, const char* file, int line, const char* func )
{
	++mem::new_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	return mem::Malloc( size, &owner );
}

// new[]
void* operator new[]( size_t size, const char* file, int line, const char* func )
{
	++mem::new_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	return mem::Malloc( size, &owner );
}

// delete
void operator delete( void* p, const char* file, int line, const char* func )
{
	++mem::delete_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	mem::Free( p, &owner );
}

// delete []
void operator delete[]( void* p, const char* file, int line, const char* func )
{
	++mem::delete_called_num;
	mem::mblock_owner_t owner = {file, line, func};
	mem::Free( p, &owner );
}

#endif // _USE_MEM_MANAGER_
