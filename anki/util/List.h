// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/Forward.h>
#include <functional>

namespace anki
{

/// @addtogroup util_containers
/// @{

namespace detail
{

/// List node.
/// @internal
template<typename T>
class ListNode
{
public:
	T m_value;
	ListNode* m_prev = nullptr;
	ListNode* m_next = nullptr;

	template<typename... TArgs>
	ListNode(TArgs&&... args)
		: m_value(std::forward<TArgs>(args)...)
	{
	}

	T& getListNodeValue()
	{
		return m_value;
	}

	const T& getListNodeValue() const
	{
		return m_value;
	}
};

/// List bidirectional iterator.
/// @internal
template<typename TNodePointer, typename TValuePointer, typename TValueReference, typename TListPointer>
class ListIterator
{
	template<typename, typename>
	friend class ListBase;

	template<typename>
	friend class anki::List;

	template<typename, typename, typename, typename>
	friend class ListIterator;

public:
	ListIterator() = default;

	ListIterator(const ListIterator& b)
		: m_node(b.m_node)
		, m_list(b.m_list)
	{
	}

	/// Allow conversion from iterator to const iterator.
	template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YList>
	ListIterator(const ListIterator<YNodePointer, YValuePointer, YValueReference, YList>& b)
		: m_node(b.m_node)
		, m_list(b.m_list)
	{
	}

	ListIterator(TNodePointer node, TListPointer list)
		: m_node(node)
		, m_list(list)
	{
		ANKI_ASSERT(list);
	}

	ListIterator& operator=(const ListIterator& b)
	{
		m_node = b.m_node;
		m_list = b.m_list;
		return *this;
	}

	TValueReference operator*() const
	{
		ANKI_ASSERT(m_node);
		return m_node->getListNodeValue();
	}

	TValuePointer operator->() const
	{
		ANKI_ASSERT(m_node);
		return &m_node->getListNodeValue();
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

	template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YList>
	Bool operator==(const ListIterator<YNodePointer, YValuePointer, YValueReference, YList>& b) const
	{
		ANKI_ASSERT(m_list == b.m_list && "Comparing iterators from different lists");
		return m_node == b.m_node;
	}

	template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YList>
	Bool operator!=(const ListIterator<YNodePointer, YValuePointer, YValueReference, YList>& b) const
	{
		return !(*this == b);
	}

private:
	TNodePointer m_node = nullptr;
	TListPointer m_list = nullptr; ///< Used to go back from the end
};

/// Double linked list base.
/// @internal
template<typename T, typename TNode>
class ListBase : public NonCopyable
{
	template<typename, typename, typename, typename>
	friend class ListIterator;

public:
	using Value = T;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Pointer = Value*;
	using ConstPointer = const Value*;
	using Iterator = ListIterator<TNode*, Pointer, Reference, ListBase*>;
	using ConstIterator = ListIterator<const TNode*, ConstPointer, ConstReference, const ListBase*>;

	ListBase() = default;

	/// Compare with another list.
	Bool operator==(const ListBase& b) const;

	/// Get first element.
	ConstReference getFront() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_head->getListNodeValue();
	}

	/// Get first element.
	Reference getFront()
	{
		ANKI_ASSERT(!isEmpty());
		return m_head->getListNodeValue();
	}

	/// Get last element.
	ConstReference getBack() const
	{
		ANKI_ASSERT(!isEmpty());
		return m_tail->getListNodeValue();
	}

	/// Get last element.
	Reference getBack()
	{
		ANKI_ASSERT(!isEmpty());
		return m_tail->getListNodeValue();
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

	/// Iterate the list using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateForward(TFunc func);

	/// Iterate the list backwards using lambda.
	template<typename TFunc>
	ANKI_USE_RESULT Error iterateBackward(TFunc func);

	/// Find item.
	Iterator find(const Value& a);

	/// Sort the list.
	/// @note It's almost 300 slower than std::list::sort, at some point replace the algorithm.
	template<typename TCompFunc = std::less<Value>>
	void sort(TCompFunc compFunc = TCompFunc());

	/// Compute the size of elements in the list.
	PtrSize getSize() const;

protected:
	TNode* m_head = nullptr;
	TNode* m_tail = nullptr;

	void move(ListBase& b)
	{
		m_head = b.m_head;
		b.m_head = nullptr;
		m_tail = b.m_tail;
		b.m_tail = nullptr;
	}

	void pushBackNode(TNode* node);
	void pushFrontNode(TNode* node);
	void insertNode(TNode* pos, TNode* node);
	void removeNode(TNode* node);
	void popBack();
	void popFront();

private:
	/// Used in sort.
	TNode* swap(TNode* one, TNode* two);
};

} // end namespace detail

/// Double linked list.
template<typename T>
class List : public detail::ListBase<T, detail::ListNode<T>>
{
private:
	using Base = detail::ListBase<T, detail::ListNode<T>>;
	using Node = detail::ListNode<T>;

public:
	using typename Base::Iterator;

	/// Default constructor.
	List()
		: Base()
	{
	}

	/// Move.
	List(List&& b)
		: List()
	{
		move(b);
	}

	/// You need to manually destroy the list. @see List::destroy
	~List()
	{
		ANKI_ASSERT(Base::isEmpty() && "Requires manual destruction");
	}

	/// Move.
	List& operator=(List&& b)
	{
		move(b);
		return *this;
	}

	/// Destroy the list.
	template<typename TAllocator>
	void destroy(TAllocator alloc);

	/// Copy an element at the end of the list.
	template<typename TAllocator>
	Iterator pushBack(TAllocator alloc, const T& x)
	{
		Node* node = alloc.template newInstance<Node>(x);
		Base::pushBackNode(node);
		return Iterator(node, this);
	}

	/// Construct an element at the end of the list.
	template<typename TAllocator, typename... TArgs>
	Iterator emplaceBack(TAllocator alloc, TArgs&&... args)
	{
		Node* node = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
		Base::pushBackNode(node);
		return Iterator(node, this);
	}

	/// Copy an element at the beginning of the list.
	template<typename TAllocator>
	Iterator pushFront(TAllocator alloc, const T& x)
	{
		Node* node = alloc.template newInstance<Node>(x);
		Base::pushFrontNode(node);
		return Iterator(node, this);
	}

	/// Construct element at the beginning of the list.
	template<typename TAllocator, typename... TArgs>
	Iterator emplaceFront(TAllocator alloc, TArgs&&... args)
	{
		Node* node = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
		Base::pushFrontNode(node);
		return Iterator(node, this);
	}

	/// Copy an element at the given position of the list.
	template<typename TAllocator>
	Iterator insert(TAllocator alloc, Iterator pos, const T& x)
	{
		Node* node = alloc.template newInstance<Node>(x);
		Base::insertNode(pos.m_node, node);
		return Iterator(node, this);
	}

	/// Construct element at the the given position.
	template<typename TAllocator, typename... TArgs>
	Iterator emplace(TAllocator alloc, Iterator pos, TArgs&&... args)
	{
		Node* node = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
		Base::insertNode(pos.m_node, node);
		return Iterator(node, this);
	}

	/// Pop a value from the back of the list.
	template<typename TAllocator>
	void popBack(TAllocator alloc)
	{
		ANKI_ASSERT(Base::m_tail);
		Node* node = Base::m_tail;
		Base::popBack();
		alloc.deleteInstance(node);
	}

	/// Pop a value from the front of the list.
	template<typename TAllocator>
	void popFront(TAllocator alloc)
	{
		ANKI_ASSERT(Base::m_head);
		Node* node = Base::m_head;
		Base::popFront();
		alloc.deleteInstance(node);
	}

	/// Erase an element.
	template<typename TAllocator>
	void erase(TAllocator alloc, Iterator pos)
	{
		ANKI_ASSERT(pos.m_node);
		ANKI_ASSERT(pos.m_list == this);
		Base::removeNode(pos.m_node);
		alloc.deleteInstance(pos.m_node);
	}

private:
	void move(List& b)
	{
		ANKI_ASSERT(Base::isEmpty() && "Requires manual destruction");
		Base::move(b);
	}
};

/// List with automatic destruction.
template<typename T>
class ListAuto : public List<T>
{
public:
	using Base = List<T>;
	using Value = T;
	using typename Base::Iterator;

	/// Construct using an allocator.
	ListAuto(GenericMemoryPoolAllocator<T> alloc)
		: Base()
		, m_alloc(alloc)
	{
	}

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
	Iterator pushBack(const Value& x)
	{
		return Base::pushBack(m_alloc, x);
	}

	/// Construct an element at the end of the list.
	template<typename... TArgs>
	Iterator emplaceBack(TArgs&&... args)
	{
		return Base::emplaceBack(m_alloc, std::forward<TArgs>(args)...);
	}

	/// Construct element at the beginning of the list.
	template<typename... TArgs>
	Iterator emplaceFront(TArgs&&... args)
	{
		return Base::emplaceFront(m_alloc, std::forward<TArgs>(args)...);
	}

	/// Construct element at the the given position.
	template<typename... TArgs>
	Iterator emplace(Iterator pos, TArgs&&... args)
	{
		return Base::emplace(m_alloc, pos, std::forward(args)...);
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

	/// Destroy the list.
	void destroy()
	{
		Base::destroy(m_alloc);
	}

private:
	GenericMemoryPoolAllocator<T> m_alloc;

	void move(ListAuto& b)
	{
		Base::move(b);
		m_alloc = b.m_alloc;
	}
};

/// The classes that will use the IntrusiveList need to inherit from this one.
template<typename TClass>
class IntrusiveListEnabled
{
	template<typename, typename, typename, typename>
	friend class detail::ListIterator;

	template<typename, typename>
	friend class detail::ListBase;

	template<typename>
	friend class List;

	template<typename>
	friend class IntrusiveList;

	friend TClass;

private:
	TClass* m_prev;
	TClass* m_next;

	IntrusiveListEnabled()
		: m_prev(nullptr)
		, m_next(nullptr)
	{
	}

	TClass& getListNodeValue()
	{
		return *static_cast<TClass*>(this);
	}

	const TClass& getListNodeValue() const
	{
		return *static_cast<const TClass*>(this);
	}
};

/// List that doesn't perform any allocations. To work the T nodes will have to inherit from IntrusiveListEnabled.
template<typename T>
class IntrusiveList : public detail::ListBase<T, T>
{
	template<typename, typename, typename, typename>
	friend class detail::ListIterator;

private:
	using Base = detail::ListBase<T, T>;

public:
	using typename Base::Iterator;

	/// Default constructor.
	IntrusiveList()
		: Base()
	{
	}

	/// Move.
	IntrusiveList(IntrusiveList&& b)
		: IntrusiveList()
	{
		Base::move(b);
	}

	~IntrusiveList() = default;

	/// Move.
	IntrusiveList& operator=(IntrusiveList&& b)
	{
		Base::move(b);
		return *this;
	}

	/// Copy an element at the end of the list.
	Iterator pushBack(T* x)
	{
		Base::pushBackNode(x);
		return Iterator(x, this);
	}

	/// Copy an element at the beginning of the list.
	Iterator pushFront(T* x)
	{
		Base::pushFrontNode(x);
		return Iterator(x, this);
	}

	/// Copy an element at the given position of the list.
	Iterator insert(Iterator pos, T* x)
	{
		Base::insertNode(pos.m_node, x);
		return Iterator(x, this);
	}

	/// Pop a value from the back of the list.
	T* popBack()
	{
		T* tail = Base::m_tail;
		Base::popBack();
		return tail;
	}

	/// Pop a value from the front of the list.
	T* popFront()
	{
		T* head = Base::m_head;
		Base::popFront();
		return head;
	}

	/// Erase an element.
	void erase(Iterator pos)
	{
		Base::removeNode(pos.m_node);
	}

	/// Erase an element.
	void erase(T* x)
	{
		Base::removeNode(x);
	}
};
/// @}

} // end namespace anki

#include <anki/util/List.inl.h>
