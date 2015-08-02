// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"

namespace anki {

// Forward
template<typename T>
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
		: m_value(std::forward<TArgs>(args)...)
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
		: m_node(b.m_node)
		, m_list(b.m_list)
	{}

	/// Allow conversion from iterator to const iterator.
	template<typename YNodePointer, typename YValuePointer,
		typename YValueReference, typename YList>
	ListIterator(const ListIterator<YNodePointer,
		YValuePointer, YValueReference, YList>& b)
		: m_node(b.m_node)
		, m_list(b.m_list)
	{}

	ListIterator(TNodePointer node, TListPointer list)
		: m_node(node)
		, m_list(list)
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
template<typename T>
class List: public NonCopyable
{
	template<typename TNodePointer, typename TValuePointer,
		typename TValueReference, typename TListPointer>
	friend class ListIterator;

public:
	using Value = T;
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
		: List()
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
	template<typename TAllocator>
	void destroy(TAllocator alloc);

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
		return Iterator(nullptr, this);
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		return ConstIterator(nullptr, this);
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
	template<typename TAllocator>
	void pushBack(TAllocator alloc, const Value& x)
	{
		Node* node = alloc.template newInstance<Node>(x);
		pushBackNode(node);
	}

	/// Construct an element at the end of the list.
	template<typename TAllocator, typename... TArgs>
	void emplaceBack(TAllocator alloc, TArgs&&... args)
	{
		Node* node = alloc.template newInstance<Node>(
			std::forward<TArgs>(args)...);
		pushBackNode(node);
	}

	/// Construct element at the beginning of the list.
	template<typename TAllocator, typename... TArgs>
	void emplaceFront(TAllocator alloc, TArgs&&... args);

	/// Construct element at the the given position.
	template<typename TAllocator, typename... TArgs>
	void emplace(TAllocator alloc, Iterator pos, TArgs&&... args);

	/// Pop a value from the back of the list.
	template<typename TAllocator>
	void popBack(TAllocator alloc);

	/// Pop a value from the front of the list.
	template<typename TAllocator>
	void popFront(TAllocator alloc);

	/// Erase an element.
	template<typename TAllocator>
	void erase(TAllocator alloc, Iterator position);

	/// Iterate the list using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateForward(TFunc func);

	/// Iterate the list backwards using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateBackward(TFunc func);

	/// Find item.
	Iterator find(const Value& a);

	/// Sort the list.
	/// @note It's almost 300 slower than std::list::sort, at some point replace
	///       the algorithm.
	template<typename TCompFunc = std::less<Value>>
	void sort(TCompFunc compFunc = TCompFunc());

	/// Compute the size of elements in the list.
	PtrSize getSize() const;

protected:
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

	/// Used in sort.
	Node* swap(Node* one, Node* two);

	void pushBackNode(Node* node);
};

/// List with automatic destruction.
template<typename T>
class ListAuto: public List<T>
{
public:
	using Base = List<T>;
	using Value = T;
	using typename Base::Iterator;

	/// Construct using an allocator.
	ListAuto(GenericMemoryPoolAllocator<T> alloc)
		: Base()
		, m_alloc(alloc)
	{}

	/// Move.
	ListAuto(ListAuto&& b)
		: Base()
	{
		move(b);
	}

	/// Destroy.
	~ListAuto()
	{
		Base::destroy(m_alloc);
	}

	/// Move.
	ListAuto& operator=(ListAuto&& b)
	{
		move(b);
		return *this;
	}

	/// Copy an element at the end of the list.
	void pushBack(const Value& x)
	{
		Base::pushBack(m_alloc, x);
	}

	/// Construct an element at the end of the list.
	template<typename... TArgs>
	void emplaceBack(TArgs&&... args)
	{
		Base::emplaceBack(m_alloc, std::forward(args)...);
	}

	/// Construct element at the beginning of the list.
	template<typename... TArgs>
	void emplaceFront(TArgs&&... args)
	{
		Base::emplaceFront(m_alloc, std::forward(args)...);
	}

	/// Construct element at the the given position.
	template<typename... TArgs>
	void emplace(Iterator pos, TArgs&&... args)
	{
		Base::emplace(m_alloc, pos, std::forward(args)...);
	}

	/// Pop a value from the back of the list.
	void popBack()
	{
		Base::popBack(m_alloc);
	}

	/// Pop a value from the front of the list.
	void popFront()
	{
		Base::popFront(m_alloc);
	}

	/// Erase an element.
	void erase(Iterator position)
	{
		Base::erase(m_alloc, position);
	}

private:
	GenericMemoryPoolAllocator<T> m_alloc;

	void move(ListAuto& b)
	{
		Base::move(b);
		m_alloc = b.m_alloc;
	}
};
/// @}

} // end namespace anki

#include "anki/util/List.inl.h"

