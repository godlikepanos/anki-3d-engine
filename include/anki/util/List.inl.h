// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki {

//==============================================================================
template<typename T, typename TAlloc>
template<typename... TArgs>
Error List<T, TAlloc>::emplaceBack(Allocator alloc, TArgs&&... args)
{
	Error err = ErrorCode::NONE;
	
	Node* el = alloc.template newInstance<Node>(
		std::forward<TArgs>(args)...);
	if(el != nullptr)
	{
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

	Node* el = alloc.template newInstance<Node>(
		std::forward<TArgs>(args)...);
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

} // end namespace anki

