#ifndef ANKI_UTIL_ALLOCATOR_H
#define ANKI_UTIL_ALLOCATOR_H

#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include <cstddef> // For ptrdiff_t
#include <cstring> // For memset
#include <atomic>

#define ANKI_DEBUG_ALLOCATORS ANKI_DEBUG
#define ANKI_PRINT_ALLOCATOR_MESSAGES 0

namespace anki {

namespace detail {

/// Static methods for the #Allocator class
struct AllocatorStatic
{
	static PtrSize allocatedSize;

	/// Print a few debugging messages
	static void dump();

	/// Allocate memory
	static void* malloc(PtrSize size);

	/// Free memory
	static void free(void* p, PtrSize size);
};

} // end namespace detail

/// The default allocator. It uses malloc and free for 
/// allocations/deallocations. It's STL compatible
template<typename T>
class Allocator
{
public:
	// Typedefs
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	/// Default constructor
	Allocator() throw()
	{}
	/// Copy constructor
	Allocator(const Allocator&) throw()
	{}
	/// Copy constructor with another type
	template<typename U>
	Allocator(const Allocator<U>&) throw()
	{}

	/// Destructor
	~Allocator()
	{}

	/// Copy
	Allocator<T>& operator=(const Allocator&)
	{
		return *this;
	}
	/// Copy with another type
	template<typename U>
	Allocator& operator=(const Allocator<U>&) 
	{
		return *this;
	}

	/// Get address of reference
	pointer address(reference x) const 
	{
		return &x; 
	}
	/// Get const address of const reference
	const_pointer address(const_reference x) const 
	{
		return &x;
	}

	/// Allocate memory
	pointer allocate(size_type n, const void* = 0)
	{
		size_type size = n * sizeof(value_type);
		return (pointer)detail::AllocatorStatic::malloc(size);
	}

	/// Deallocate memory
	void deallocate(void* p, size_type n)
	{
		size_type size = n * sizeof(T);
		detail::AllocatorStatic::free(p, size);
	}

	/// Call constructor
	void construct(pointer p, const T& val)
	{
		// Placement new
		new ((T*)p) T(val); 
	}
	/// Call constructor with more arguments
	template<typename U, typename... Args>
	void construct(U* p, Args&&... args)
	{
		// Placement new
		::new((void*)p) U(std::forward<Args>(args)...);
	}

	/// Call the destructor of p
	void destroy(pointer p) 
	{
		p->~T();
	}
	/// Call the destructor of p of type U
	template<typename U>
	void destroy(U* p)
	{
		p->~U();
	}

	/// Get the max allocation size
	size_type max_size() const 
	{
		return size_type(-1); 
	}

	/// A struct to rebind the allocator to another allocator of type U
	template<typename U>
	struct rebind
	{ 
		typedef Allocator<U> other; 
	};
};

/// Another allocator of the same type can deallocate from this one
template<typename T1, typename T2>
inline bool operator==(const Allocator<T1>&, const Allocator<T2>&)
{
	return true;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator>
inline bool operator==(const Allocator<T1>&, const AnotherAllocator&)
{
	return false;
}

namespace detail {

/// Thread safe memory pool
struct MemoryPool
{
	/// Allocated memory
	U8* memory = nullptr;
	/// Size of the allocated memory
	PtrSize size = 0;
	/// Points to the memory and more specifically to the address of the next
	/// allocation
	std::atomic<U8*> ptr = {nullptr};
	/// Reference counter
	std::atomic<I32> refCounter = {1};
};

/// Internal members for the StackAllocator. They are separate because we don't
/// want to polute the StackAllocator template with specialized functions that
/// take space
class StackAllocatorInternal
{
protected:
	/// The memory pool
	detail::MemoryPool* mpool = nullptr;

	/// Init the memory pool with the given size
	void init(const PtrSize size);

	/// Deinit the memory pool
	void deinit();
};

} // end namespace detail

/// Stack based allocator
///
/// @tparam T The type
/// @tparam deallocationFlag If true then the allocator will try to deallocate
///                          the memory. This is extremely important to
///                          understand when it should be true. See notes
/// @tparam alignmentBits Set the alighment in bits
///
/// @note The deallocationFlag can brake the allocator when the deallocations
///       are not in the correct order. For example when deallocationFlag==true
///       and the allocator is used in vector it is likely to fail
///
/// @note Don't ever EVER remove the double copy constructor and the double
///       operator=. The compiler will create defaults
template<typename T, Bool deallocationFlag = false, U32 alignmentBits = 16>
class StackAllocator: public detail::StackAllocatorInternal
{
	template<typename U, Bool deallocationFlag_, U32 alignmentBits_>
	friend class StackAllocator;

public:
	// Typedefs
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	/// Default constructor deleted
	StackAllocator() = delete;
	/// Copy constructor
	StackAllocator(const StackAllocator& b) throw()
	{
		*this = b;
	}
	/// Copy constructor
	template<typename U>
	StackAllocator(
		const StackAllocator<U, deallocationFlag, alignmentBits>& b) throw()
	{
		*this = b;
	}
	/// Constuctor with size
	StackAllocator(size_type size) throw()
	{
		init(size);
	}

	/// Destructor
	~StackAllocator()
	{
		deinit();
	}

	/// Copy
	StackAllocator& operator=(const StackAllocator& b)
	{
		mpool = b.mpool;
		// Retain the mpool
		++mpool->refCounter;
		return *this;
	}
	/// Copy
	template<typename U>
	StackAllocator& operator=(
		const StackAllocator<U, deallocationFlag, alignmentBits>& b)
	{
		mpool = b.mpool;
		// Retain the mpool
		++mpool->refCounter;
		return *this;
	}

	/// Get the address of a reference
	pointer address(reference x) const
	{
		return &x;
	}
	/// Get the const address of a const reference
	const_pointer address(const_reference x) const
	{
		return &x;
	}

	/// Allocate memory
	pointer allocate(size_type n, const void* hint = 0)
	{
		(void)hint;
		size_type size = n * sizeof(value_type);
		size_type alignedSize = calcAlignSize(size);

		U8* out = mpool->ptr.fetch_add(alignedSize);

#if ANKI_PRINT_ALLOCATOR_MESSAGES
			std::cout << "Allocating: size: " << size
				<< ", size after alignment: " << alignedSize
				<< ", returned address: " << out
				<< ", hint: " << hint <<std::endl;
#endif

		if(out + alignedSize <= mpool->memory + mpool->size)
		{
			// Everything ok
		}
		else
		{
			throw ANKI_EXCEPTION("Allocation failed. There is not enough room");
		}

		return (pointer)out;
	}

	/// Deallocate memory
	void deallocate(void* p, size_type n)
	{
		(void)p;
		(void)n;

		if(deallocationFlag)
		{
			size_type alignedSize = calcAlignSize(n * sizeof(value_type));
#if ANKI_PRINT_ALLOCATOR_MESSAGES
			std::cout << "Deallocating: size: " << (n * sizeof(value_type))
				<< " alignedSize: " << alignedSize
				<< " pointer: " << p << std::endl;
#endif
			U8* headPtr = mpool->ptr.fetch_sub(alignedSize);

			if(headPtr - alignedSize != p)
			{
				throw ANKI_EXCEPTION("Freeing wrong pointer. "
					"The deallocations on StackAllocator should be in order");
			}

			ANKI_ASSERT((headPtr - alignedSize) >= mpool->memory);
		}
	}

	/// Call constructor
	void construct(pointer p, const T& val)
	{
		// Placement new
		new ((T*)p) T(val);
	}
	/// Call constructor with many arguments
	template<typename U, typename... Args>
	void construct(U* p, Args&&... args)
	{
		// Placement new
		::new((void *)p) U(std::forward<Args>(args)...);
	}

	/// Call destructor
	void destroy(pointer p)
	{
		p->~T();
	}
	/// Call destructor
	template<typename U>
	void destroy(U* p)
	{
		p->~U();
	}

	/// Get the max allocation size
	size_type max_size() const
	{
		return mpool->size;
	}

	template<typename U>
	struct rebind
	{
		typedef StackAllocator<U, deallocationFlag, alignmentBits> other;
	};

	void reset()
	{
		mpool->ptr = mpool->memory;
	}

private:
	/// Calculate tha align size
	size_type calcAlignSize(size_type size)
	{
		return size + (size % (alignmentBits / 8));
	}
};

/// Another allocator of the same type can deallocate from this one
template<typename T1, typename T2, Bool deallocationFlag, U32 alignmentBits>
inline bool operator==(
	const StackAllocator<T1, deallocationFlag, alignmentBits>&,
	const StackAllocator<T2, deallocationFlag, alignmentBits>&)
{
	return true;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator, Bool deallocationFlag,
	U32 alignmentBits>
inline bool operator==(
	const StackAllocator<T1, deallocationFlag, alignmentBits>&,
	const AnotherAllocator&)
{
	return false;
}

} // end namespace anki

#endif
