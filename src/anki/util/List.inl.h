// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki
{
namespace detail
{

template<typename TNodePointer, typename TValuePointer, typename TValueReference, typename TList>
ListIterator<TNodePointer, TValuePointer, TValueReference, TList>&
	ListIterator<TNodePointer, TValuePointer, TValueReference, TList>::operator--()
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

template<typename T, typename TNode>
Bool ListBase<T, TNode>::operator==(const ListBase& b) const
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

template<typename T, typename TNode>
void ListBase<T, TNode>::pushBackNode(TNode* node)
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

template<typename T, typename TNode>
void ListBase<T, TNode>::pushFrontNode(TNode* node)
{
	ANKI_ASSERT(node);

	if(m_head != nullptr)
	{
		ANKI_ASSERT(m_tail != nullptr);
		m_head->m_prev = node;
		node->m_next = m_head;
		m_head = node;
	}
	else
	{
		ANKI_ASSERT(m_tail == nullptr);
		m_tail = m_head = node;
	}
}

template<typename T, typename TNode>
void ListBase<T, TNode>::insertNode(TNode* pos, TNode* node)
{
	ANKI_ASSERT(node);

	if(pos == nullptr)
	{
		// Place after the last

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
	else
	{
		node->m_prev = pos->m_prev;
		node->m_next = pos;
		pos->m_prev = node;

		if(pos == m_head)
		{
			ANKI_ASSERT(m_tail != nullptr);
			m_head = node;
		}
	}
}

template<typename T, typename TNode>
template<typename TFunc>
Error ListBase<T, TNode>::iterateForward(TFunc func)
{
	Error err = ErrorCode::NONE;
	TNode* node = m_head;
	while(node && !err)
	{
		err = func(node->getListNodeValue());
		node = node->m_next;
	}

	return err;
}

template<typename T, typename TNode>
template<typename TFunc>
Error ListBase<T, TNode>::iterateBackward(TFunc func)
{
	Error err = ErrorCode::NONE;
	TNode* node = m_tail;
	while(node && !err)
	{
		err = func(node->getListNodeValue());
		node = node->m_prev;
	}

	return err;
}

template<typename T, typename TNode>
template<typename TCompFunc>
void ListBase<T, TNode>::sort(TCompFunc compFunc)
{
	TNode* sortPtr;
	TNode* newTail = m_tail;

	while(newTail != m_head)
	{
		sortPtr = m_head;
		Bool swapped = false;
		Bool finished = false;

		do
		{
			ANKI_ASSERT(sortPtr != nullptr);
			TNode* sortPtrNext = sortPtr->m_next;
			ANKI_ASSERT(sortPtrNext != nullptr);

			if(compFunc(sortPtrNext->getListNodeValue(), sortPtr->getListNodeValue()))
			{
				sortPtr = swap(sortPtr, sortPtrNext);
				swapped = true;
			}
			else
			{
				sortPtr = sortPtrNext;
			}

			if(sortPtr == m_tail || sortPtr == newTail)
			{
				if(swapped)
				{
					newTail = sortPtr->m_prev;
				}
				else
				{
					newTail = m_head;
				}

				finished = true;
			}
		} while(!finished);
	}
}

template<typename T, typename TNode>
TNode* ListBase<T, TNode>::swap(TNode* one, TNode* two)
{
	if(one->m_prev == nullptr)
	{
		m_head = two;
	}
	else
	{
		ANKI_ASSERT(one->m_prev);
		one->m_prev->m_next = two;
	}

	if(two->m_next == nullptr)
	{
		m_tail = one;
	}
	else
	{
		ANKI_ASSERT(two->m_next);
		two->m_next->m_prev = one;
	}

	two->m_prev = one->m_prev;
	one->m_next = two->m_next;
	one->m_prev = two;
	two->m_next = one;

	return one;
}

template<typename T, typename TNode>
void ListBase<T, TNode>::removeNode(TNode* node)
{
	ANKI_ASSERT(node);
	if(node != m_head)
	{
		ANKI_ASSERT(!(node->m_prev == nullptr && node->m_next == nullptr));
	}

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

	node->m_next = node->m_prev = nullptr;
}

template<typename T, typename TNode>
typename ListBase<T, TNode>::Iterator ListBase<T, TNode>::find(const Value& a)
{
	Iterator it = getBegin();
	Iterator endit = getEnd();
	while(it != endit)
	{
		if(*it == a)
		{
			break;
		}

		++it;
	}

	return it;
}

template<typename T, typename TNode>
PtrSize ListBase<T, TNode>::getSize() const
{
	PtrSize size = 0;
	ConstIterator it = getBegin();
	ConstIterator endit = getEnd();
	for(; it != endit; it++)
	{
		++size;
	}

	return size;
}

template<typename T, typename TNode>
void ListBase<T, TNode>::popBack()
{
	ANKI_ASSERT(m_tail);
	removeNode(m_tail);
}

template<typename T, typename TNode>
void ListBase<T, TNode>::popFront()
{
	ANKI_ASSERT(m_head);
	removeNode(m_head);
}

} // end namespace detail

template<typename T>
template<typename TAllocator>
void List<T>::destroy(TAllocator alloc)
{
	Node* el = Base::m_head;
	while(el)
	{
		Node* next = el->m_next;
		alloc.deleteInstance(el);
		el = next;
	}

	Base::m_head = Base::m_tail = nullptr;
}

} // end namespace anki
