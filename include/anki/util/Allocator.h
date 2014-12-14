// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_ALLOCATOR_H
#define ANKI_UTIL_ALLOCATOR_H

#include "anki/util/Assert.h"
#include "anki/util/Memory.h"
#include "anki/util/Logger.h"
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
///
/// @note Don't ever EVER remove the double copy constructor and the double
///       operator=. The compiler will create defaults
template<typename T, typename TPool>
class GenericPoolAllocator
{
	template<typename Y, typename TPool_>
	friend class GenericPoolAllocator;

public:
	// Typedefs
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	using value_type = T;

	/// Move assignments between containers will copy the allocator as well. 
	/// If propagate_on_container_move_assignment is not defined then not moves
	/// are going to happen
	using propagate_on_container_move_assignment = std::true_type;

	/// A struct to rebind the allocator to another allocator of type U
	template<typename Y>
	struct rebind
	{
		using other = GenericPoolAllocator<Y, TPool>;
	};

	/// Default constructor
	GenericPoolAllocator()
	{}

	/// Copy constructor
	GenericPoolAllocator(const GenericPoolAllocator& b)
	{
		*this = b;
	}

	/// Copy constructor
	template<typename Y>
	GenericPoolAllocator(const GenericPoolAllocator<Y, TPool>& b)
	{
		*this = b;
	}

	/// Constuctor that creates a pool
	template<typename... TArgs>
	explicit GenericPoolAllocator(
		AllocAlignedCallback allocCb, void* allocCbUserData,
		TArgs&&... args)
	{
		m_pool = reinterpret_cast<TPool*>(
			allocCb(allocCbUserData, nullptr, sizeof(TPool), alignof(TPool)));
		if(!m_pool)
		{
			ANKI_LOGF("Initialization failed");
		}

		new (m_pool) TPool();

		Error error = m_pool->create(
			allocCb, allocCbUserData, std::forward<TArgs>(args)...);

		if(error)
		{
			ANKI_LOGF("Initialization failed");
		}

		m_pool->getRefcount() = 1;
	}

	/// Destructor
	~GenericPoolAllocator()
	{
		clear();
	}

	/// Copy
	GenericPoolAllocator& operator=(const GenericPoolAllocator& b)
	{
		copy(b);
		return *this;
	}

	/// Copy
	template<typename U>
	GenericPoolAllocator& operator=(const GenericPoolAllocator<U, TPool>& b)
	{
		copy(b);
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

		void* out = m_pool->allocate(size, alignment);

		if(out == nullptr)
		{
			ANKI_LOGE("Allocation failed. There is not enough room");
		}

		return reinterpret_cast<pointer>(out);
	}

	/// Deallocate memory
	void deallocate(void* p, size_type n)
	{
		(void)n;
		m_pool->free(p);
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
		ANKI_ASSERT(p != nullptr);
		p->~T();
	}

	/// Call destructor
	template<typename U>
	void destroy(U* p)
	{
		ANKI_ASSERT(p != nullptr);
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
		ANKI_ASSERT(m_pool);
		return *m_pool;
	}

	/// Get the memory pool
	/// @note This is AnKi specific
	TPool& getMemoryPool()
	{
		ANKI_ASSERT(m_pool);
		return *m_pool;
	}

	/// Allocate a new object and call it's constructor
	/// @note This is AnKi specific
	template<typename U, typename... Args>
	U* newInstance(Args&&... args)
	{
		typename rebind<U>::other alloc(*this);

		U* ptr = alloc.allocate(1);
		if(ptr)
		{
			alloc.construct(ptr, std::forward<Args>(args)...);
		}

		return ptr;
	}

	/// Allocate a new array of objects and call their constructor
	/// @note This is AnKi specific
	template<typename U>
	U* newArray(size_type n)
	{
		typename rebind<U>::other alloc(*this);

		U* ptr = alloc.allocate(n);
		if(ptr)
		{
			// Call the constuctors
			for(size_type i = 0; i < n; i++)
			{
				alloc.construct(&ptr[i]);
			}
		}

		return ptr;
	}

	/// Allocate a new array of objects and call their constructor
	/// @note This is AnKi specific
	template<typename U>
	U* newArray(size_type n, const U& v)
	{
		typename rebind<U>::other alloc(*this);

		U* ptr = alloc.allocate(n);
		if(ptr)
		{
			// Call the constuctors
			for(size_type i = 0; i < n; i++)
			{
				alloc.construct(&ptr[i], v);
			}
		}

		return ptr;
	}

	/// Call the destructor and deallocate an object
	/// @note This is AnKi specific
	template<typename U>
	void deleteInstance(U* ptr)
	{
		typename rebind<U>::other alloc(*this);

		if(ptr != nullptr)
		{
			alloc.destroy(ptr);
			alloc.deallocate(ptr, 1);
		}
	}

	/// Call the destructor and deallocate an array of objects
	/// @note This is AnKi specific
	template<typename U>
	void deleteArray(U* ptr, size_type n)
	{
		typename rebind<U>::other alloc(*this);

		if(ptr != nullptr)
		{
			// Call the destructors
			for(size_type i = 0; i < n; i++)
			{
				alloc.destroy(&ptr[i]);
			}

			alloc.deallocate(ptr, n);
		}
		else
		{
			ANKI_ASSERT(n == 0);
		}
	}

private:
	TPool* m_pool = nullptr;

	template<typename Y>
	void copy(const GenericPoolAllocator<Y, TPool>& b)
	{
		clear();
		if(b.m_pool)
		{
			m_pool = b.m_pool;
			++m_pool->getRefcount();
		}
	}

	void clear()
	{
		if(m_pool)
		{
			auto count = --m_pool->getRefcount();
			if(count == 0)
			{
				auto allocCb = m_pool->getAllocationCallback();
				auto ud = m_pool->getAllocationCallbackUserData();
				ANKI_ASSERT(allocCb);
				m_pool->~TPool();
				allocCb(ud, m_pool, 0, 0);
			}

			m_pool = nullptr;
		}
	}
};

/// @name GenericPoolAllocator global functions
/// @{

/// Another allocator of the same type can deallocate from this one
template<typename T1, typename T2, typename TPool>
inline Bool operator==(
	const GenericPoolAllocator<T1, TPool>&,
	const GenericPoolAllocator<T2, TPool>&)
{
	return true;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator, typename TPool>
inline Bool operator==(
	const GenericPoolAllocator<T1, TPool>&,
	const AnotherAllocator&)
{
	return false;
}

/// Another allocator of the same type can deallocate from this one
template<typename T1, typename T2, typename TPool>
inline Bool operator!=(
	const GenericPoolAllocator<T1, TPool>&,
	const GenericPoolAllocator<T2, TPool>&)
{
	return false;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator, typename TPool>
inline Bool operator!=(
	const GenericPoolAllocator<T1, TPool>&,
	const AnotherAllocator&)
{
	return true;
}
/// @}

/// Allocator using the base memory pool.
template<typename T>
using GenericMemoryPoolAllocator = GenericPoolAllocator<T, BaseMemoryPool>;

/// Heap based allocator. The default allocator. It uses malloc and free for 
/// allocations/deallocations
template<typename T>
using HeapAllocator = GenericPoolAllocator<T, HeapMemoryPool>;

/// Allocator that uses a StackMemoryPool
template<typename T>
using StackAllocator = GenericPoolAllocator<T, StackMemoryPool>;

/// Allocator that uses a ChainMemoryPool
template<typename T>
using ChainAllocator = GenericPoolAllocator<T, ChainMemoryPool>;
/// @}

} // end namespace anki

#endif
