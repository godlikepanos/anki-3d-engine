// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Assert.h>
#include <anki/util/Memory.h>
#include <anki/util/Logger.h>
#include <anki/util/Ptr.h>
#include <cstddef> // For ptrdiff_t
#include <utility> // For forward
#include <new> // For placement new

namespace anki
{

/// @addtogroup util_memory
/// @{

/// Pool based allocator
///
/// This is a template that accepts memory pools with a specific interface
///
/// @tparam T The type
///
/// @note Don't ever EVER remove the double copy constructor and the double operator=. The compiler will create defaults
template<typename T, typename TPool>
class GenericPoolAllocator
{
	template<typename, typename>
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

	/// Move assignments between containers will copy the allocator as well. If propagate_on_container_move_assignment
	/// is not defined then not moves are going to happen.
	using propagate_on_container_move_assignment = std::true_type;

	/// A struct to rebind the allocator to another allocator of type Y
	template<typename Y>
	struct rebind
	{
		using other = GenericPoolAllocator<Y, TPool>;
	};

	/// Default constructor
	GenericPoolAllocator()
	{
	}

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

	/// Copy constructor, uses another type of allocator
	template<typename Y, typename YPool>
	GenericPoolAllocator(const GenericPoolAllocator<Y, YPool>& b)
	{
		auto balloc = b;
		m_pool = &balloc.getMemoryPool();
		m_pool->getRefcount().fetchAdd(1);
	}

	/// Constuctor that creates a pool
	template<typename... TArgs>
	explicit GenericPoolAllocator(AllocAlignedCallback allocCb, void* allocCbUserData, TArgs&&... args)
	{
		m_pool = static_cast<TPool*>(allocCb(allocCbUserData, nullptr, sizeof(TPool), alignof(TPool)));
		if(ANKI_UNLIKELY(!m_pool))
		{
			ANKI_UTIL_LOGF("Out of memory");
		}

		::new(m_pool) TPool();

		m_pool->create(allocCb, allocCbUserData, std::forward<TArgs>(args)...);
		m_pool->getRefcount().store(1);
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
	template<typename Y>
	GenericPoolAllocator& operator=(const GenericPoolAllocator<Y, TPool>& b)
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
	/// @param n The elements of type T to allocate
	/// @param hint It's been used to override the alignment. The type should be PtrSize.
	pointer allocate(size_type n, const void* hint = nullptr)
	{
		ANKI_ASSERT(m_pool);
		(void)hint;

		size_type size = n * sizeof(value_type);

		// Operator new doesn't respect alignment (in GCC at least) so use the type's alignment. If hint override the
		// alignment
		PtrSize alignment = (hint != nullptr) ? *static_cast<const PtrSize*>(hint) : alignof(value_type);

		void* out = m_pool->allocate(size, alignment);
		if(ANKI_UNLIKELY(out == nullptr))
		{
			ANKI_UTIL_LOGF("Out of memory");
		}

		return static_cast<pointer>(out);
	}

	/// Deallocate memory
	void deallocate(void* p, size_type n)
	{
		ANKI_ASSERT(m_pool);
		(void)n;
		m_pool->free(p);
	}

	/// Call constructor
	void construct(pointer p, const T& val)
	{
		// Placement new
		::new(p) T(val);
	}

	/// Call constructor with many arguments
	template<typename Y, typename... Args>
	void construct(Y* p, Args&&... args)
	{
		// Placement new
		::new(static_cast<void*>(p)) Y(std::forward<Args>(args)...);
	}

	/// Call destructor
	void destroy(pointer p)
	{
		static_assert(sizeof(T) > 0, "Incomplete type");
		ANKI_ASSERT(p != nullptr);
		p->~T();
	}

	/// Call destructor
	template<typename Y>
	void destroy(Y* p)
	{
		static_assert(sizeof(T) > 0, "Incomplete type");
		ANKI_ASSERT(p != nullptr);
		p->~Y();
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
	template<typename Y, typename... Args>
	Y* newInstance(Args&&... args)
	{
		typename rebind<Y>::other alloc(*this);

		Y* ptr = alloc.allocate(1);
		if(ptr)
		{
			alloc.construct(ptr, std::forward<Args>(args)...);
		}

		return ptr;
	}

	/// Allocate a new array of objects and call their constructor
	/// @note This is AnKi specific
	template<typename Y>
	Y* newArray(size_type n)
	{
		typename rebind<Y>::other alloc(*this);

		Y* ptr = alloc.allocate(n);
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
	template<typename Y>
	Y* newArray(size_type n, const Y& v)
	{
		typename rebind<Y>::other alloc(*this);

		Y* ptr = alloc.allocate(n);
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
	template<typename Y>
	void deleteInstance(Y* ptr)
	{
		if(ptr != nullptr)
		{
			typename rebind<Y>::other alloc(*this);
			alloc.destroy(ptr);
			alloc.deallocate(ptr, 1);
		}
	}

	/// Call the destructor and deallocate an object
	/// @note This is AnKi specific
	template<typename Y>
	void deleteInstance(WeakPtr<Y> ptr)
	{
		if(ptr)
		{
			typename rebind<Y>::other alloc(*this);
			alloc.destroy(&ptr[0]);
			alloc.deallocate(&ptr[0], 1);
		}
	}

	/// Call the destructor and deallocate an array of objects
	/// @note This is AnKi specific
	template<typename Y>
	void deleteArray(Y* ptr, size_type n)
	{
		typename rebind<Y>::other alloc(*this);

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
			m_pool->getRefcount().fetchAdd(1);
		}
	}

	void clear()
	{
		if(m_pool)
		{
			auto count = m_pool->getRefcount().fetchSub(1);
			if(count == 1)
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
inline Bool operator==(const GenericPoolAllocator<T1, TPool>&, const GenericPoolAllocator<T2, TPool>&)
{
	return true;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator, typename TPool>
inline Bool operator==(const GenericPoolAllocator<T1, TPool>&, const AnotherAllocator&)
{
	return false;
}

/// Another allocator of the same type can deallocate from this one
template<typename T1, typename T2, typename TPool>
inline Bool operator!=(const GenericPoolAllocator<T1, TPool>&, const GenericPoolAllocator<T2, TPool>&)
{
	return false;
}

/// Another allocator of the another type cannot deallocate from this one
template<typename T1, typename AnotherAllocator, typename TPool>
inline Bool operator!=(const GenericPoolAllocator<T1, TPool>&, const AnotherAllocator&)
{
	return true;
}
/// @}

/// Allocator using the base memory pool.
template<typename T>
using GenericMemoryPoolAllocator = GenericPoolAllocator<T, BaseMemoryPool>;

/// Heap based allocator. The default allocator. It uses malloc and free for allocations/deallocations
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
