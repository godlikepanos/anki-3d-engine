// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki {

//==============================================================================
// ListIterator                                                                =
//==============================================================================

//==============================================================================
template<typename TNodePointer, typename TValuePointer, 
	typename TValueReference, typename TList>
ListIterator<TNodePointer, TValuePointer, TValueReference, TList>& 
	ListIterator<TNodePointer, TValuePointer, TValueReference, TList>::
	operator--()
{
	ANKI_ASSERT(m_list);

	if(m_node)
	{
		m_node = m_node->m_prev;
	}
	else
	{
		m_node = m_list->m_tail;
	}

	return *this;
}

//==============================================================================
// List                                                                        =
//==============================================================================

//==============================================================================
template<typename T, typename TAlloc>
Bool List<T, TAlloc>::operator==(const List& b) const
{
	Bool same = true;
	ConstIterator ita = getBegin();
	ConstIterator itb = b.getBegin();

	while(same && ita != getEnd() && itb != b.getEnd())
	{
		same = *ita == *itb;
		++ita;
		++itb;
	}

	if(same && (ita != getEnd() || itb != b.getEnd()))
	{
		same = false;
	}

	return same;
}

//==============================================================================
template<typename T, typename TAlloc>
void List<T, TAlloc>::pushBackNode(Node* node)
{
	ANKI_ASSERT(node);

	if(m_tail != nullptr)
	{
		ANKI_ASSERT(m_head != nullptr);
		m_tail->m_next = node;
		node->m_prev = m_tail;
		m_tail = node;
	}
	else
	{
		ANKI_ASSERT(m_head == nullptr);
		m_tail = m_head = node;
	}
}

//==============================================================================
template<typename T, typename TAlloc>
Error List<T, TAlloc>::pushBack(Allocator alloc, const Value& x)
{
	Error err = ErrorCode::NONE;
	
	Node* node = alloc.template newInstance<Node>(x);
	if(node != nullptr)
	{
		pushBackNode(node);
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename... TArgs>
Error List<T, TAlloc>::emplaceBack(Allocator alloc, TArgs&&... args)
{
	Error err = ErrorCode::NONE;
	
	Node* node = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
	if(node != nullptr)
	{
		pushBackNode(node);
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename... TArgs>
Error List<T, TAlloc>::emplaceFront(Allocator alloc, TArgs&&... args)
{
	Error err = ErrorCode::NONE;

	Node* el = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
	if(el != nullptr)
	{
		if(m_head != nullptr)
		{
			ANKI_ASSERT(m_tail != nullptr);
			m_head->m_prev = el;
			el->m_next = m_head;
			m_head = el;
		}
		else
		{
			ANKI_ASSERT(m_tail == nullptr);
			m_tail = m_head = el;
		}
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename... TArgs>
Error List<T, TAlloc>::emplace(Allocator alloc, Iterator pos, TArgs&&... args)
{
	ANKI_ASSERT(pos.m_list == this);
	Error err = ErrorCode::NONE;

	Node* el = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
	if(el != nullptr)
	{
		Node* node = pos.m_node;

		if(node == nullptr)
		{
			// Place after the last

			if(m_tail != nullptr)
			{
				ANKI_ASSERT(m_head != nullptr);
				m_tail->m_next = el;
				el->m_prev = m_tail;
				m_tail = el;
			}
			else
			{
				ANKI_ASSERT(m_head == nullptr);
				m_tail = m_head = el;
			}
		}
		else
		{
			el->m_prev = node->m_prev;
			el->m_next = node;
			node->m_prev = el;

			if(node == m_head)
			{
				ANKI_ASSERT(m_tail != nullptr);
				m_head = el;
			}
		}
	}
	else
	{
		err = ErrorCode::OUT_OF_MEMORY;
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc>
void List<T, TAlloc>::destroy(Allocator alloc)
{
	Node* el = m_head;
	while(el)
	{
		Node* next = el->m_next;
		alloc.deleteInstance(el);
		el = next;
	}

	m_head = m_tail = nullptr;
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename TFunc>
Error List<T, TAlloc>::iterateForward(TFunc func)
{
	Error err = ErrorCode::NONE;
	Node* el = m_head;
	while(el && !err)
	{
		err = func(el->m_value);
		el = el->m_next;
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename TFunc>
Error List<T, TAlloc>::iterateBackward(TFunc func)
{
	Error err = ErrorCode::NONE;
	Node* el = m_tail;
	while(el && !err)
	{
		err = func(el->m_value);
		el = el->m_prev;
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename TCompFunc>
void List<T, TAlloc>::sort(TCompFunc compFunc)
{
	sortInternal(compFunc, m_head, m_tail);
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename TCompFunc>
void List<T, TAlloc>::sortInternal(TCompFunc compFunc, Node* l, Node* r)
{
	if (r != nullptr && l != r && l != r->m_next)
	{
		Node* p = partition(compFunc, l, r);
		sortInternal(compFunc, l, p->m_prev);
		sortInternal(compFunc, p->m_next, r);
	}
}

//==============================================================================
template<typename T, typename TAlloc>
template<typename TCompFunc>
typename List<T, TAlloc>::Node* List<T, TAlloc>::partition(
	TCompFunc compFunc, Node* l, Node* r)
{
	// Set pivot as h element
	Value& x = r->m_value;
 
	// similar to i = l-1 for array implementation
	Node* i = l->m_prev;
 
	// Similar to "for (int j = l; j <= h- 1; j++)"
	for(Node* j = l; j != r; j = j->m_next)
	{
		if(compFunc(j->m_value, x))
		{
			// Similar to i++ for array
			i = (i == nullptr) ? l : i->m_next;

			// Swap
			std::swap(i->m_value, j->m_value);
		}
	}

	i = (i == nullptr) ? l : i->m_next; // Similar to i++
	
	std::swap(i->m_value, r->m_value);

	return i;
}

//==============================================================================
template<typename T, typename TAlloc>
void List<T, TAlloc>::erase(Allocator alloc, Iterator pos)
{
	ANKI_ASSERT(pos.m_node);
	ANKI_ASSERT(pos.m_list == this);

	Node* node = pos.m_node;
	
	if(node == m_tail)
	{
		m_tail = node->m_prev;
	}

	if(node == m_head)
	{
		m_head = node->m_next;
	}

	if(node->m_prev)
	{
		ANKI_ASSERT(node->m_prev->m_next == node);
		node->m_prev->m_next = node->m_next;
	}

	if(node->m_next)
	{
		ANKI_ASSERT(node->m_next->m_prev == node);
		node->m_next->m_prev = node->m_prev;
	}

	alloc.deleteInstance(node);
}

//==============================================================================
template<typename T, typename TAlloc>
void List<T, TAlloc>::popBack(Allocator alloc)
{
	ANKI_ASSERT(m_tail);
	erase(alloc, Iterator(m_tail, this));
}

//==============================================================================
template<typename T, typename TAlloc>
typename List<T, TAlloc>::Iterator List<T, TAlloc>::find(const Value& a)
{
	Iterator it = getBegin();
	Iterator end = getEnd();
	while(it != end)
	{
		if(*it == a)
		{
			break;
		}

		++it;
	}

	return it;
}

//==============================================================================
template<typename T, typename TAlloc>
PtrSize List<T, TAlloc>::getSize() const
{
	PtrSize size = 0;
	ConstIterator it = getBegin();
	ConstIterator end = getEnd();
	for(; it != end; it++)
	{
		++size;
	}

	return size;
}

} // end namespace anki

