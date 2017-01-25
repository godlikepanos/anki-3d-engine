// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki
{
namespace detail
{

template<typename TKey, typename TValue, typename THasher, typename TCompare, typename TNode>
void HashMapBase<TKey, TValue, THasher, TCompare, TNode>::insertNode(TNode* node)
{
	if(ANKI_UNLIKELY(!m_root))
	{
		m_root = node;
		return;
	}

	const U64 hash = node->m_hash;
	TNode* it = m_root;
	Bool done = false;
	do
	{
		const U64 nhash = it->m_hash;
		if(hash > nhash)
		{
			// Go to right
			TNode* right = it->m_right;
			if(right)
			{
				it = right;
			}
			else
			{
				node->m_parent = it;
				it->m_right = node;
				done = true;
			}
		}
		else
		{
			ANKI_ASSERT(hash != nhash && "Not supported");
			// Go to left
			TNode* left = it->m_left;
			if(left)
			{
				it = left;
			}
			else
			{
				node->m_parent = it;
				it->m_left = node;
				done = true;
			}
		}
	} while(!done);
}

template<typename TKey, typename TValue, typename THasher, typename TCompare, typename TNode>
typename HashMapBase<TKey, TValue, THasher, TCompare, TNode>::Iterator
HashMapBase<TKey, TValue, THasher, TCompare, TNode>::find(const Key& key)
{
	const U64 hash = THasher()(key);

	TNode* node = m_root;
	while(node)
	{
		const U64 bhash = node->m_hash;

		if(hash < bhash)
		{
			node = node->m_left;
		}
		else if(hash > bhash)
		{
			node = node->m_right;
		}
		else
		{
			// Found
			break;
		}
	}

	return Iterator(node);
}

template<typename TKey, typename TValue, typename THasher, typename TCompare, typename TNode>
typename HashMapBase<TKey, TValue, THasher, TCompare, TNode>::ConstIterator
HashMapBase<TKey, TValue, THasher, TCompare, TNode>::find(const Key& key) const
{
	const U64 hash = THasher()(key);

	const TNode* node = m_root;
	while(node)
	{
		const U64 bhash = node->m_hash;

		if(hash < bhash)
		{
			node = node->m_left;
		}
		else if(hash > bhash)
		{
			node = node->m_right;
		}
		else
		{
			// Found
			break;
		}
	}

	return ConstIterator(node);
}

template<typename TKey, typename TValue, typename THasher, typename TCompare, typename TNode>
void HashMapBase<TKey, TValue, THasher, TCompare, TNode>::removeNode(TNode* del)
{
	ANKI_ASSERT(del);
	TNode* parent = del->m_parent;
	TNode* left = del->m_left;
	TNode* right = del->m_right;

	if(parent)
	{
		// If it has a parent then remove the connection to the parent and insert left and right like regular nodes

		if(parent->m_left == del)
		{
			parent->m_left = nullptr;
		}
		else
		{
			ANKI_ASSERT(parent->m_right == del);
			parent->m_right = nullptr;
		}

		if(left)
		{
			ANKI_ASSERT(left->m_parent == del);
			left->m_parent = nullptr;
			insertNode(left);
		}

		if(right)
		{
			ANKI_ASSERT(right->m_parent == del);
			right->m_parent = nullptr;
			insertNode(right);
		}
	}
	else
	{
		// It's the root node. Make arbitrarily the left root and add the right

		ANKI_ASSERT(del == m_root && "It must be the root");

		if(left)
		{
			left->m_parent = nullptr;
			m_root = left;

			if(right)
			{
				right->m_parent = nullptr;
				insertNode(right);
			}
		}
		else
		{
			if(right)
			{
				right->m_parent = nullptr;
			}

			m_root = right;
		}
	}
}

} // end namespace detail

template<typename TKey, typename TValue, typename THasher, typename TCompare>
template<typename TAllocator>
void HashMap<TKey, TValue, THasher, TCompare>::destroy(TAllocator alloc)
{
	if(Base::m_root)
	{
		destroyInternal(alloc, Base::m_root);
		Base::m_root = nullptr;
	}
}

template<typename TKey, typename TValue, typename THasher, typename TCompare>
template<typename TAllocator>
void HashMap<TKey, TValue, THasher, TCompare>::destroyInternal(TAllocator alloc, Node* node)
{
	ANKI_ASSERT(node);

	if(node->m_right)
	{
		destroyInternal(alloc, node->m_right);
	}

	if(node->m_left)
	{
		destroyInternal(alloc, node->m_left);
	}

	alloc.deleteInstance(node);
}

} // end namespace anki
