#ifndef ANKI_UTIL_MEMORY_H
#define ANKI_UTIL_MEMORY_H

#include "anki/util/StdTypes.h"
#include "anki/util/Assert.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Functions.h"
#include <atomic>
#include <algorithm>

namespace anki {

// Forward
template<typename T>
class Allocator;

/// @addtogroup util
/// @{
/// @addtogroup memory
/// @{

/// Thread safe memory pool
class StackMemoryPool: public NonCopyable
{
public:
	/// Default constructor
	StackMemoryPool(PtrSize size, U32 alignmentBits = 16);

	/// Move
	StackMemoryPool(StackMemoryPool&& other)
	{
		*this = std::move(other);
	}

	/// Destroy
	~StackMemoryPool();

	/// Move
	StackMemoryPool& operator=(StackMemoryPool&& other);

	/// Access the total size
	PtrSize getSize() const
	{
		return memsize;
	}

	/// Get the allocated size
	PtrSize getAllocatedSize() const;

	/// Allocate memory
	void* allocate(PtrSize size) throw();

	/// Free memory in StackMemoryPool. If the ptr is not the last allocation
	/// then nothing happens and the method returns false
	Bool free(void* ptr) throw();

	/// Reinit the pool. All existing allocated memory will be lost
	void reset();

private:
	/// Pre-allocated memory memory chunk
	U8* memory = nullptr;

	/// Size of the allocated memory chunk
	PtrSize memsize = 0;

	/// Points to the memory and more specifically to the top of the stack
	std::atomic<U8*> top = {nullptr};

	/// Alignment
	U32 alignmentBits;

	/// Calculate tha aligned size of an allocation
	PtrSize calcAlignSize(PtrSize size) const;
};

/// Functior that imitates the new operator. The functior allocates memory for
/// a number of elements and calls their constructor. The interesting thing is
/// that if the elements size is >1 then it allocates size bigger than the
/// required. The extra chunk is a number that will be used in
/// deleteObjectArray to identify the number of elements that were allocated
template<typename T, typename Alloc = Allocator<T>>
struct New
{
	template<typename... Args>
	T* operator()(PtrSize n, Alloc alloc, Args&&... args)
	{
		ANKI_ASSERT(n != 0);
		T* out;

		// If the number of elements is then do a simple allocaton
		if(n == 1)
		{
			out = alloc.allocate(n);
		}
		else
		{
			// Allocate a memory block that includes the array size
			typedef typename Alloc::template rebind<U8>::other CharAlloc;
			CharAlloc charAlloc(alloc);
			U8* mem = charAlloc.allocate(sizeof(PtrSize) + n * sizeof(T));

			// Set the size of the block
			*(PtrSize*)mem = n;

			// Set the output address
			out = (T*)(mem + sizeof(PtrSize));
		}

		// Call the constuctors
		for(PtrSize i = 0; i < n; i++)
		{
			alloc.construct(&out[i], std::forward<Args>(args)...);
		}

		// Return result
		return out;
	}
};

/// Function that imitates the delete operator
template<typename T, typename Alloc = Allocator<T>>
struct Delete
{
	Alloc alloc;

	Delete(const Alloc& alloc_ = Alloc())
		: alloc(alloc_)
	{}

	void operator()(void* ptr)
	{
		T* p = (T*)ptr;

		// Make sure the type is defined
		typedef U8 TypeMustBeComplete[sizeof(T) ? 1 : -1];
		(void) sizeof(TypeMustBeComplete);

		if(p)
		{
			// Call the destructor
			alloc.destroy(p);

			// Deallocate
			alloc.deallocate(p, 1);
		}
	}
};

/// Function that imitates the delete[] operator
template<typename T, typename Alloc = Allocator<T>>
struct DeleteArray
{
	Alloc alloc;

	DeleteArray(const Alloc& alloc_ = Alloc())
		: alloc(alloc_)
	{}

	void operator()(void* ptr)
	{
		// Make sure the type is defined
		typedef U8 TypeMustBeComplete[sizeof(T) ? 1 : -1];
		(void) sizeof(TypeMustBeComplete);

		T* p = (T*)ptr;

		if(p)
		{
			// Get the allocated block
			U8* block = (U8*)(p) - sizeof(PtrSize);

			// Get number of elements
			const PtrSize n = *(PtrSize*)block;

			// Call the destructors
			for(PtrSize i = 0; i < n; i++)
			{
				alloc.destroy(&p[i]);
			}

			// Deallocate the block
			typename Alloc::template rebind<U8>::other allocc(alloc);
			allocc.deallocate(block, n * sizeof(T) + sizeof(PtrSize));

			// nullify
			ptr = nullptr;
		}
	}
};

/// Allocate memory using an allocator
#define ANKI_NEW(Type_, alloc_, ...) \
	New<Type_, decltype(alloc_)::rebind<Type_>::other>{}( \
		1, alloc_, ## __VA_ARGS__)

/// Allocate memory using an allocator
#define ANKI_NEW_0(Type_, alloc_) \
	New<Type_, decltype(alloc_)::rebind<Type_>::other>{}( \
		1, alloc_)

/// Allocate memory using an allocator
#define ANKI_NEW_ARRAY(Type_, alloc_, n_, ...) \
	New<Type_, decltype(alloc_)::rebind<Type_>::other>{}( \
		n_, alloc_, ## __VA_ARGS__)

/// Allocate memory using an allocator
#define ANKI_NEW_ARRAY_0(Type_, alloc_, n_) \
	New<Type_, decltype(alloc_)::rebind<Type_>::other>{}( \
		n_, alloc_)

/// Delete memory allocated by #ANKI_NEW
#define ANKI_DELETE(ptr_, alloc_) \
	Delete<RemovePointer<decltype(ptr_)>::Type, \
		decltype(alloc_)::rebind<RemovePointer<decltype(ptr_)>::Type>::other \
		>{alloc_}(ptr_);

/// Delete memory allocated by #ANKI_NEW_ARRAY
#define ANKI_DELETE_ARRAY(ptr_, alloc_) \
	DeleteArray<RemovePointer<decltype(ptr_)>::Type, \
		decltype(alloc_)::rebind<RemovePointer<decltype(ptr_)>::Type>::other \
		>{alloc_}(ptr_);

/// @}
/// @}

} // end namespace anki

#endif
