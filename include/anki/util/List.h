// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_LIST_H
#define ANKI_UTIL_LIST_H

#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"

namespace anki {

// Forward
template<typename T, typename TAlloc>
class List;

/// @addtogroup util_private
/// @{

/// List node.
template<typename T>
class ListNode
{
public:
	using Value = T;

	T m_value;
	ListNode* m_prev = nullptr;
	ListNode* m_next = nullptr;

	template<typename... TArgs>
	ListNode(TArgs&&... args)
	:	m_value(std::forward<TArgs>(args)...)
	{}
};

/// List bidirectional iterator.
template<typename TNodePointer, typename TValuePointer, 
	typename TValueReference, typename TListPointer>
class ListIterator
{
public:
	TNodePointer m_node = nullptr;
	TListPointer m_list = nullptr; ///< Used to go back from the end

	ListIterator() = default;

	ListIterator(const ListIterator& b)
	:	m_node(b.m_node),
		m_list(b.m_list)
	{}

	/// Allow conversion from iterator to const iterator.
	template<typename YNodePointer, typename YValuePointer, 
		typename YValueReference, typename YList>
	ListIterator(const ListIterator<YNodePointer, 
		YValuePointer, YValueReference, YList>& b)
	:	m_node(b.m_node),
		m_list(b.m_list)
	{}

	ListIterator(TNodePointer node, TListPointer list)
	:	m_node(node),
		m_list(list)
	{
		ANKI_ASSERT(list);
	}

	TValueReference operator*() const
	{
		ANKI_ASSERT(m_node);
		return m_node->m_value;
	}

	TValuePointer operator->() const
	{
		ANKI_ASSERT(m_node);
		return &m_node->m_value;
	}

	ListIterator& operator++()
	{
		ANKI_ASSERT(m_node);
		m_node = m_node->m_next;
		return *this;
	}

	ListIterator operator++(int)
	{
		ANKI_ASSERT(m_node);
		ListIterator out = *this;
		++(*this);
		return out;
	}

	ListIterator& operator--();

	ListIterator operator--(int)
	{
		ANKI_ASSERT(m_node);
		ListIterator out = *this;
		--(*this);
		return out;
	}

	ListIterator operator+(U n) const
	{
		ListIterator it = *this;
		while(n-- != 0)
		{
			++it;
		}
		return it;
	}

	ListIterator operator-(U n) const
	{
		ListIterator it = *this;
		while(n-- != 0)
		{
			--it;
		}
		return it;
	}

	ListIterator& operator+=(U n)
	{
		while(n-- != 0)
		{
			++(*this);
		}
		return *this;
	}

	ListIterator& operator-=(U n)
	{
		while(n-- != 0)
		{
			--(*this);
		}
		return *this;
	}

	Bool operator==(const ListIterator& b) const
	{
		ANKI_ASSERT(m_list == b.m_list 
			&& "Comparing iterators from different lists");
		return m_node == b.m_node;
	}

	Bool operator!=(const ListIterator& b) const
	{
		return !(*this == b);
	}
};

/// @}

/// @addtogroup util_containers
/// @{

/// Double linked list.
template<typename T, typename TAlloc = HeapAllocator<T>>
class List: public NonCopyable
{
	template<typename TNodePointer, typename TValuePointer, 
		typename TValueReference, typename TListPointer>
	friend class ListIterator;

public:
	using Value = T;
	using Allocator = TAlloc;
	using Node = ListNode<Value>;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Pointer = Value*;
	using ConstPointer = const Value*;
	using Iterator = ListIterator<Node*, Pointer, Reference, List*>;
	using ConstIterator = 
		ListIterator<const Node*, ConstPointer, ConstReference, const List*>;

	List() = default;

	/// Move.
	List(List&& b)
	:	List()
	{
		move(b);
	}

	/// You need to manually destroy the list.
	/// @see List::destroy
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

	/// Compare with another list.
	Bool operator==(const List& b) const;

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

	/// Get begin.
	Iterator getBegin()
	{
		return Iterator(m_head, this);
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return ConstIterator(m_head, this);
	}

	/// Get end.
	Iterator getEnd()
	{
		Iterator it(nullptr, this);
		return it;
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		ConstIterator it(nullptr, this);
		return it;
	}

	/// Get begin.
	Iterator begin()
	{
		return getBegin();
	}

	/// Get begin.
	ConstIterator begin() const
	{
		return getBegin();
	}

	/// Get end.
	Iterator end()
	{
		return getEnd();
	}

	/// Get end.
	ConstIterator end() const
	{
		return getEnd();
	}

	/// Return true if list is empty.
	Bool isEmpty() const
	{
		return m_head == nullptr;
	}

	/// Copy an element at the end of the list.
	ANKI_USE_RESULT Error pushBack(Allocator alloc, const Value& x);

	/// Construct an element at the end of the list.
	template<typename... TArgs>
	ANKI_USE_RESULT Error emplaceBack(Allocator alloc, TArgs&&... args);

	/// Construct element at the beginning of the list.
	template<typename... TArgs>
	ANKI_USE_RESULT Error emplaceFront(Allocator alloc, TArgs&&... args);

	/// Construct element at the the given position.
	template<typename... TArgs>
	ANKI_USE_RESULT Error emplace(
		Allocator alloc, Iterator pos, TArgs&&... args);

	/// Pop a value from the back of the list.
	void popBack(Allocator alloc);

	/// Erase an element.
	void erase(Allocator alloc, Iterator position);

	/// Iterate the list using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateForward(TFunc func);

	/// Iterate the list backwards using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateBackward(TFunc func);

	/// Find item.
	Iterator find(const Value& a);

	/// Quicksort.
	template<typename TCompFunc = std::less<Value>>
	void sort(TCompFunc compFunc = TCompFunc());

	/// Compute the size of elements in the list.
	PtrSize getSize() const;

private:
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

	void pushBackNode(Node* node);
};

/// @}

} // end namespace anki

#include "anki/util/List.inl.h"

#endif

