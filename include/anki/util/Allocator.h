// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_ALLOCATOR_H
#define ANKI_UTIL_ALLOCATOR_H

#include "anki/util/Exception.h"
#include "anki/util/Assert.h"
#include "anki/util/Memory.h"
#include <cstddef> // For ptrdiff_t
#include <utility> // For forward

#define ANKI_DEBUG_ALLOCATORS ANKI_DEBUG
#define ANKI_PRINT_ALLOCATOR_MESSAGES 1

#if ANKI_PRINT_ALLOCATOR_MESSAGES
#	include <iostream> // Never include it on release
#endif

namespace anki {

/// @addtogroup util_memory
/// @{

/// Pool based allocator
///
/// This is a template that accepts memory pools with a specific interface
///
/// @tparam T The type
/// @tparam deallocationFlag If true then the allocator will try to deallocate
///                          the memory. It is extremely important to
///                          understand when it should be true. See notes
///
/// @note The deallocationFlag can brake the allocator when used with stack 
///       pools and the deallocations are not in the correct order.
///
/// @note Don't ever EVER remove the double copy constructor and the double
///       operator=. The compiler will create defaults
template<typename T, typename TPool, Bool deallocationFlag = false>
class GenericPoolAllocator
{
	template<typename Y, typename TPool_, Bool deallocationFlag_>
	friend class GenericPoolAllocator;

public:
	// Typedefs
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	/// Move assignments between containers will copy the allocator as well. 
	/// If propagate_on_container_move_assignment is not defined then not moves
	/// are going to happen
	typedef std::true_type propagate_on_container_move_assignment;

	/// A struct to rebind the allocator to another allocator of type U
	template<typename Y>
	struct rebind
	{
		typedef GenericPoolAllocator<Y, TPool, deallocationFlag> other;
	};

	/// Default constructor
	GenericPoolAllocator() noexcept
	{}

	/// Copy constructor
	GenericPoolAllocator(const GenericPoolAllocator& b) noexcept
	{
		*this = b;
	}

	/// Copy constructor
	template<typename Y>
	GenericPoolAllocator(const GenericPoolAllocator<
		Y, TPool, deallocationFlag>& b) noexcept
	{
		*this = b;
	}

	/// Constuctor that accepts a pool
	explicit GenericPoolAllocator(const TPool& pool) noexcept
	:	m_pool(pool)
	{}

	/// Destructor
	~GenericPoolAllocator()
	{}

	/// Copy
	GenericPoolAllocator& operator=(const GenericPoolAllocator& b)
	{
		m_pool = b.m_pool;
		return *this;
	}

	/// Copy
	template<typename U>
	GenericPoolAllocator& operator=(const GenericPoolAllocator<
		U, TPool, deallocationFlag>& b)
	{
		m_pool = b.m_pool;
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
	/// @param n The bytes to allocate
	/// @param hint It's been used to override the alignment. The type should
	///             be PtrSize
	pointer allocate(size_type n, const void* hint = nullptr)
	{
		(void)hint;

		size_type size = n * sizeof(value_type);

		// Operator new doesn't respect alignment (in GCC at least) so use 
		// the types alignment. If hint override the alignment
		PtrSize alignment = (hint != nullptr) 
			? *reinterpret_cast<const PtrSize*>(hint) 
			: alignof(value_type);

		void* out = m_pool.allocate(size, alignment);

		if(out == nullptr)
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
			Bool ok = m_pool.free(p);

			if(!ok)
			{
				throw ANKI_EXCEPTION("Freeing wrong pointer. "
					"Pool's free returned false");
			}
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
		return MAX_PTR_SIZE;
	}

	/// Get the memory pool
	/// @note This is AnKi specific
	const TPool& getMemoryPool() const
	{
		return m_pool;
	}

	/// Get the memory pool
	/// @note This is AnKi specific
	TPool& getMemoryPool()
	{
		return m_pool;
	}

	/// Allocate a new object and call it's constructor
	/// @note This is AnKi specific
	template<typename U, typename... Args>
	U* newInstance(Args&&... args)
	{
		typename rebind<U>::other alloc(*this);

		U* x = alloc.allocate(1);
		alloc.construct(x, std::forward<Args>(args)...);
		return x;
	}

	/// Allocate a new array of objects and call their constructor
	/// @note This is AnKi specific
	template<typename U, typename... Args>
	U* newArray(size_type n, Args&&... args)
	{
		typename rebind<U>::other alloc(*this);

		U* x = alloc.allocate(n);
		// Call the constuctors
		for(size_type i = 0; i < n; i++)
		{
			alloc.construct(&x[i], std::forward<Args>(args)...);
		}
		return x;
	}

	/// Call the destructor and deallocate an object
	/// @note This is AnKi specific
	template<typename U>
	void deleteInstance(U* x)
	{
		typename rebind<U>::other alloc(*this);

		alloc.destroy(x);
		alloc.deallocate(x, 1);
	}

	/// Call the destructor and deallocate an array of objects
	/// @note This is AnKi specific
	template<typename U>
	void deleteArray(U* x, size_type n)
	{
		typename rebind<U>::other alloc(*this);

		// Call the destructors
		for(size_type i = 0; i < n; i++)
		{
			alloc.destroy(&x[i]);
		}
		alloc.deallocate(x, n);
	}

private:
	TPool m_pool;
};

/// @name GenericPoolAllocator global functions
/// @{

/// Another allocator of the same type can deallocate from this one
template<typename T1, typename T2, typename TPool, Bool deallocationFlag>
inline bool operator==(
	const GenericPoolAllocator<T1, TPool, deallocationFlag>&,
	const GenericPoolAllocator<T2, TPool, deallocationFlag>&)
{
	return true;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator, typename TPool, 
	Bool deallocationFlag>
inline bool operator==(
	const GenericPoolAllocator<T1, TPool, deallocationFlag>&,
	const AnotherAllocator&)
{
	return false;
}

/// Another allocator of the same type can deallocate from this one
template<typename T1, typename T2, typename TPool, Bool deallocationFlag>
inline bool operator!=(
	const GenericPoolAllocator<T1, TPool, deallocationFlag>&,
	const GenericPoolAllocator<T2, TPool, deallocationFlag>&)
{
	return false;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator, typename TPool, 
	Bool deallocationFlag>
inline bool operator!=(
	const GenericPoolAllocator<T1, TPool, deallocationFlag>&,
	const AnotherAllocator&)
{
	return true;
}

/// @}

/// Heap based allocator. The default allocator. It uses malloc and free for 
/// allocations/deallocations
template<typename T>
using HeapAllocator = 
	GenericPoolAllocator<T, HeapMemoryPool, true>;

/// Allocator that uses a StackMemoryPool
template<typename T, Bool deallocationFlag = false>
using StackAllocator = 
	GenericPoolAllocator<T, StackMemoryPool, deallocationFlag>;

/// Allocator that uses a ChainMemoryPool
template<typename T, Bool deallocationFlag = true>
using ChainAllocator = 
	GenericPoolAllocator<T, ChainMemoryPool, deallocationFlag>;

/// @}

} // end namespace anki

#endif
