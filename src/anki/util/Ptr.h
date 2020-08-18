// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>

namespace anki
{

/// @addtogroup util_memory
/// @{

/// Common code for all pointers
template<typename T>
class PtrBase
{
	template<typename Y>
	friend class PtrBase;

public:
	/// Dereference
	T& operator*() const
	{
		ANKI_ASSERT(m_ptr);
		return *m_ptr;
	}

	/// Dereference
	T* operator->() const
	{
		ANKI_ASSERT(m_ptr);
		return m_ptr;
	}

	T* get()
	{
		ANKI_ASSERT(m_ptr);
		return m_ptr;
	}

	const T* get() const
	{
		ANKI_ASSERT(m_ptr);
		return m_ptr;
	}

	Bool isCreated() const
	{
		return m_ptr != nullptr;
	}

	/// @name Compare operators
	/// @{
	explicit operator Bool() const
	{
		return isCreated();
	}

	Bool operator==(const PtrBase& other) const
	{
		return m_ptr == other.m_ptr;
	}

	Bool operator!=(const PtrBase& other) const
	{
		return m_ptr != other.m_ptr;
	}

	Bool operator<(const PtrBase& other) const
	{
		return m_ptr < other.m_ptr;
	}

	Bool operator<=(const PtrBase& other) const
	{
		return m_ptr <= other.m_ptr;
	}

	Bool operator>(const PtrBase& other) const
	{
		return m_ptr > other.m_ptr;
	}

	Bool operator>=(const PtrBase& other) const
	{
		return m_ptr >= other.m_ptr;
	}
	/// @}

protected:
	T* m_ptr;

	PtrBase()
		: m_ptr(nullptr)
	{
	}

	PtrBase(T* ptr)
		: m_ptr(ptr)
	{
	}
};

/// A simple template class to wrap simple pointers.
template<typename T>
class WeakPtr : public PtrBase<T>
{
	template<typename>
	friend class WeakPtr;

public:
	using Base = PtrBase<T>;

	WeakPtr()
		: Base()
	{
	}

	explicit WeakPtr(T* ptr)
		: Base(ptr)
	{
	}

	WeakPtr(const WeakPtr& other)
		: Base(other.m_ptr)
	{
	}

	template<typename Y>
	WeakPtr(const WeakPtr<Y>& other)
		: Base(other.m_ptr)
	{
	}

	WeakPtr(WeakPtr&& other)
		: Base(other.m_ptr)
	{
		other.m_ptr = nullptr;
	}

	/// Destroy.
	~WeakPtr()
	{
		Base::m_ptr = nullptr;
	}

	/// Copy.
	WeakPtr& operator=(const WeakPtr& other)
	{
		Base::m_ptr = other.m_ptr;
		return *this;
	}

	/// Copy.
	template<typename Y>
	WeakPtr& operator=(const WeakPtr<Y>& other)
	{
		Base::m_ptr = other.m_ptr;
		return *this;
	}

	/// Copy.
	WeakPtr& operator=(T* ptr)
	{
		Base::m_ptr = ptr;
		return *this;
	}

	/// Copy.
	template<typename Y>
	WeakPtr& operator=(Y* ptr)
	{
		Base::m_ptr = ptr;
		return *this;
	}

	/// Move.
	WeakPtr& operator=(WeakPtr&& other)
	{
		Base::m_ptr = other.m_ptr;
		other.m_ptr = nullptr;
		return *this;
	}

	/// Cast to the pointer type.
	operator T*()
	{
		return Base::m_ptr;
	}

	/// Cast to the pointer type.
	operator const T*() const
	{
		return Base::m_ptr;
	}

	/// @name Arithmetic operators
	/// @{
	WeakPtr& operator++()
	{
		ANKI_ASSERT(Base::m_ptr);
		++Base::m_ptr;
		return *this;
	}

	WeakPtr& operator--()
	{
		ANKI_ASSERT(Base::m_ptr);
		--Base::m_ptr;
		return *this;
	}

	WeakPtr operator+(const WeakPtr& other) const
	{
		ANKI_ASSERT(Base::m_ptr);
		WeakPtr out(Base::m_ptr + other.m_ptr);
		return out;
	}

	WeakPtr& operator+=(const WeakPtr& other)
	{
		ANKI_ASSERT(Base::m_ptr);
		Base::m_ptr += other.m_ptr;
		return *this;
	}

	WeakPtr operator-(const WeakPtr& other) const
	{
		ANKI_ASSERT(Base::m_ptr);
		WeakPtr out(Base::m_ptr - other.m_ptr);
		return out;
	}

	WeakPtr& operator-=(const WeakPtr& other)
	{
		ANKI_ASSERT(Base::m_ptr);
		Base::m_ptr -= other.m_ptr;
		return *this;
	}

	T& operator[](const PtrSize i)
	{
		return Base::m_ptr[i];
	}

	const T& operator[](const PtrSize i) const
	{
		return Base::m_ptr[i];
	}
	/// @}
};

/// UniquePtr default deleter.
template<typename T>
class DefaultPtrDeleter
{
public:
	void operator()(T* x)
	{
		auto alloc = x->getAllocator();
		alloc.template deleteInstance<T>(x);
	}
};

/// UniquePtr alternative deleter.
template<typename T>
class AllocatorPtrDeleter
{
public:
	GenericMemoryPoolAllocator<U8> m_allocator;

	AllocatorPtrDeleter()
	{
	}

	AllocatorPtrDeleter(GenericMemoryPoolAllocator<U8> alloc)
		: m_allocator(alloc)
	{
	}

	void operator()(T* x)
	{
		m_allocator.template deleteInstance<T>(x);
	}
};

/// A unique pointer.
template<typename T, typename TDeleter = DefaultPtrDeleter<T>>
class UniquePtr : public PtrBase<T>
{
public:
	using Base = PtrBase<T>;
	using Deleter = TDeleter;

	UniquePtr()
		: Base()
	{
	}

	explicit UniquePtr(T* ptr, const Deleter& deleter = Deleter())
		: Base(ptr)
		, m_deleter(deleter)
	{
	}

	/// Non-copyable.
	UniquePtr(const UniquePtr& other) = delete;

	/// Move.
	UniquePtr(UniquePtr&& other)
		: Base()
	{
		move(other);
	}

	/// Destroy.
	~UniquePtr()
	{
		destroy();
	}

	/// Non-copyable.
	UniquePtr& operator=(const UniquePtr& other) = delete;

	/// Move.
	UniquePtr& operator=(UniquePtr&& other)
	{
		move(other);
		return *this;
	}

	/// Set a new pointer. Will destroy the previous.
	void reset(T* ptr, const Deleter& deleter = Deleter())
	{
		destroy();
		Base::m_ptr = ptr;
		m_deleter = deleter;
	}

	/// Move the ownership of the pointer outside the UniquePtr.
	void moveAndReset(T*& ptr)
	{
		ptr = Base::m_ptr;
		Base::m_ptr = nullptr;
		m_deleter = Deleter();
	}

private:
	Deleter m_deleter;

	void destroy()
	{
		if(Base::m_ptr)
		{
			m_deleter(Base::m_ptr);
			Base::m_ptr = nullptr;
			m_deleter = Deleter();
		}
	}

	void move(UniquePtr& b)
	{
		reset(b.m_ptr, b.m_deleter);
		b.m_ptr = nullptr;
	}
};

/// An intrusive pointer.
template<typename T, typename TDeleter = DefaultPtrDeleter<T>>
class IntrusivePtr : public PtrBase<T>
{
	template<typename Y, typename TD>
	friend class IntrusivePtr;

public:
	using Base = PtrBase<T>;
	using Deleter = TDeleter;

	IntrusivePtr()
		: Base()
	{
	}

	explicit IntrusivePtr(T* ptr)
		: Base()
	{
		reset(ptr);
	}

	/// Copy.
	IntrusivePtr(const IntrusivePtr& other)
		: Base()
	{
		reset(other.m_ptr);
	}

	/// Move.
	IntrusivePtr(IntrusivePtr&& other)
		: Base()
	{
		move(other);
	}

	/// Copy, compatible pointer.
	template<typename Y>
	IntrusivePtr(const IntrusivePtr<Y, TDeleter>& other)
		: Base()
	{
		reset(other.m_ptr);
	}

	/// Decrease refcount and delete the pointer if refcount is zero.
	~IntrusivePtr()
	{
		destroy();
	}

	/// Copy.
	IntrusivePtr& operator=(const IntrusivePtr& other)
	{
		reset(other.m_ptr);
		return *this;
	}

	/// Move.
	IntrusivePtr& operator=(IntrusivePtr&& other)
	{
		destroy();
		move(other);
		return *this;
	}

	/// Copy, compatible.
	template<typename Y, typename YDeleter>
	IntrusivePtr& operator=(const IntrusivePtr<Y, YDeleter>& other)
	{
		reset(other.m_ptr);
		return *this;
	}

	/// Set a new pointer. Will destroy the previous.
	void reset(T* ptr)
	{
		destroy();
		if(ptr)
		{
			ptr->getRefcount().fetchAdd(1);
			Base::m_ptr = ptr;
		}
	}

private:
	void destroy()
	{
		if(Base::m_ptr)
		{
			auto count = Base::m_ptr->getRefcount().fetchSub(1);
			if(ANKI_UNLIKELY(count == 1))
			{
				TDeleter deleter;
				deleter(Base::m_ptr);
			}

			Base::m_ptr = nullptr;
		}
	}

	void move(IntrusivePtr& b)
	{
		ANKI_ASSERT(Base::m_ptr == nullptr);
		Base::m_ptr = b.m_ptr;
		b.m_ptr = nullptr;
	}
};
/// @}

} // end namespace anki
