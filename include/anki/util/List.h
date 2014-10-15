// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_LIST_H
#define ANKI_UTIL_LIST_H

#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"

namespace anki {

/// @addtogroup util_containers
/// @{

/// Double linked list.
template<typename T, typename TAlloc = HeapAllocator<T>>
class List: public NonCopyable
{
public:
	using Value = T;
	using Allocator = TAlloc;
	using Reference = Value&;
	using ConstReference = const Value&;

	List() = default;

	/// Move.
	List(List&& b)
	:	List()
	{
		move(b);
	}

	~List()
	{
		ANKI_ASSERT(m_head == nullptr && "Requires manual destruction");
	}

	/// Move.
	List& operator=(List&& b)
	{
		move(b);
		return *this;
	}

	/// Destroy the list.
	void destroy(Allocator alloc);

	/// Get first element.
	ConstReference getFront() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_head->m_value;
	}

	/// Get first element.
	Reference getFront()
	{
		ANKI_ASSERT(!isEmpty());
		return m_head->m_value;
	}

	/// Get last element.
	ConstReference getBack() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_tail->m_value;
	}

	/// Get last element.
	Reference getBack()
	{
		ANKI_ASSERT(!isEmpty());
		return m_tail->m_value;
	}

	template<typename... TArgs>
	ANKI_USE_RESULT Error emplaceBack(Allocator alloc, TArgs&&... args);

	template<typename... TArgs>
	ANKI_USE_RESULT Error emplaceFront(Allocator alloc, TArgs&&... args);

	/// Iterate the list using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateForward(TFunc func);

	/// Iterate the list backwards using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateBackward(TFunc func);

	Bool isEmpty() const
	{
		return m_head == nullptr;
	}

	/// Quicksort.
	template<typename TCompFunc = std::less<Value>>
	void sort(TCompFunc compFunc = TCompFunc());

private:
	class Node
	{
	public:
		Value m_value;
		Node* m_prev = nullptr;
		Node* m_next = nullptr;

		template<typename... TArgs>
		Node(TArgs&&... args)
		:	m_value(std::forward<TArgs>(args)...)
		{}
	};

	Node* m_head = nullptr;
	Node* m_tail = nullptr;

	void move(List& b)
	{
		ANKI_ASSERT(isEmpty() && "Cannot move before destroying");
		m_head = b.m_head;
		b.m_head = nullptr;
		m_tail = b.m_tail;
		b.m_tail = nullptr;
	}

	/// Sort.
	template<typename TCompFunc>
	void sortInternal(TCompFunc compFunc, Node* l, Node* r);

	/// Used in sortInternal.
	template<typename TCompFunc>
	Node* partition(TCompFunc compFunc, Node* l, Node* r);
};

/// @}

} // end namespace anki

#include "anki/util/List.inl.h"

#endif

