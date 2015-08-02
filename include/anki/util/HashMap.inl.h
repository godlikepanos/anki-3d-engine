// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki {

//==============================================================================
template<typename T, typename THasher, typename TCompare, typename TNode>
template<typename TAllocator>
void HashMap<T, THasher, TCompare, TNode>::destroy(TAllocator alloc)
{
	if(m_root)
	{
		destroyInternal(alloc, m_root);
		m_root = nullptr;
	}
}

//==============================================================================
template<typename T, typename THasher, typename TCompare, typename TNode>
template<typename TAllocator>
void HashMap<T, THasher, TCompare, TNode>::destroyInternal(
	TAllocator alloc, Node* node)
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

//==============================================================================
template<typename T, typename THasher, typename TCompare, typename TNode>
void HashMap<T, THasher, TCompare, TNode>::pushBackNode(Node* node)
{
	if(ANKI_UNLIKELY(!m_root))
	{
		m_root = node;
		return;
	}

	const U64 hash = node->m_hash;
	Node* it = m_root;
	while(1)
	{
		const U64 nhash = it->m_hash;
		if(hash > nhash)
		{
			// Go to right
			if(it->m_right)
			{
				it = it->m_right;
			}
			else
			{
				it->m_right = node;
				node->m_parent = it;
				break;
			}
		}
		else
		{
			ANKI_ASSERT(hash != nhash && "Not supported");
			// Go to left
			if(it->m_left)
			{
				it = it->m_left;
			}
			else
			{
				it->m_left = node;
				node->m_parent = it;
				break;
			}
		}
	}
}

//==============================================================================
template<typename T, typename THasher, typename TCompare, typename TNode>
typename HashMap<T, THasher, TCompare, TNode>::Iterator
	HashMap<T, THasher, TCompare, TNode>::find(const Value& a)
{
	if(ANKI_UNLIKELY(!m_root))
	{
		Iterator(nullptr);
	}

	U64 hash = THasher()(a);
	return Iterator(findInternal(m_root, a, hash));
}

//==============================================================================
template<typename T, typename THasher, typename TCompare, typename TNode>
TNode* HashMap<T, THasher, TCompare, TNode>::findInternal(Node* node,
	const Value& a, U64 aHash)
{
	ANKI_ASSERT(node);
	if(node->m_hash == aHash)
	{
		// Found
		ANKI_ASSERT(TCompare()(node->m_value, a)
			&& "Doesn't accept collisions");
		return node;
	}

	if(aHash < node->m_hash)
	{
		node = node->m_left;
	}
	else
	{
		node = node->m_right;
	}

	if(node)
	{
		return findInternal(node, a, aHash);
	}

	return node;
}

//==============================================================================
template<typename T, typename THasher, typename TCompare, typename TNode>
template<typename TAllocator>
void HashMap<T, THasher, TCompare, TNode>::erase(TAllocator alloc, Iterator it)
{
	Node* del = it.m_node;
	ANKI_ASSERT(del);
	Node* parent = del->m_parent;
	Node* left = del->m_left;
	Node* right = del->m_right;

	alloc.deleteInstance(del);

	if(parent)
	{
		// If it has a parent then remove the connection to the parent and
		// insert left and right like regular nodes

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
			pushBackNode(left);
		}

		if(right)
		{
			ANKI_ASSERT(right->m_parent == del);
			right->m_parent = nullptr;
			pushBackNode(right);
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
				pushBackNode(right);
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

} // end namespace anki

