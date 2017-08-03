// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/SparseArray.h>

namespace anki
{

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
TNode** SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::findPlace(Index idx, Node*& parent)
{
	parent = nullptr;
	const Index modIdx = mod(idx);

	if(m_bucket.m_elements[modIdx] == nullptr || m_bucket.m_elements[modIdx]->m_saIdx == idx)
	{
		// We are lucky, found empty spot or something to replace
		return &m_bucket.m_elements[modIdx];
	}

	// Do linear probing
	Index probingIdx = modIdx + 1;
	while(probingIdx - modIdx < LINEAR_PROBING_SIZE && probingIdx < BUCKET_SIZE)
	{
		if(m_bucket.m_elements[probingIdx] == nullptr || m_bucket.m_elements[probingIdx]->m_saIdx == idx)
		{
			return &m_bucket.m_elements[probingIdx];
		}

		++probingIdx;
	}

	// Check if we can evict a node. This will happen if that node is from linear probing
	const Index otherModIdx = mod(m_bucket.m_elements[modIdx]->m_saIdx);
	if(otherModIdx != modIdx)
	{
		ANKI_ASSERT(m_bucket.m_elements[modIdx]->m_saLeft == nullptr
			&& m_bucket.m_elements[modIdx]->m_saRight == nullptr
			&& m_bucket.m_elements[modIdx]->m_saParent == nullptr
			&& "Can't be a tree");

		// Do a hack. Chage the other node's idx
		Node* const otherNode = m_bucket.m_elements[modIdx];
		const Index otherNodeIdx = otherNode->m_saIdx;
		otherNode->m_saIdx = idx;

		// ...and try to find a new place for it. If we don't do the trick above findPlace will return the same place
		Node* parent;
		Node** newPlace = findPlace(otherNodeIdx, parent);
		ANKI_ASSERT(*newPlace != m_bucket.m_elements[modIdx]);

		// ...point the other node to the new place and restore it
		*newPlace = otherNode;
		otherNode->m_saIdx = otherNodeIdx;
		otherNode->m_saParent = parent;

		// ...now the modIdx place is free for out node
		m_bucket.m_elements[modIdx] = nullptr;
		return &m_bucket.m_elements[modIdx];
	}

	// Last thing we can do, need to append to a tree
	ANKI_ASSERT(m_bucket.m_elements[modIdx]);
	Node* it = m_bucket.m_elements[modIdx];
	while(true)
	{
		if(idx > it->m_saIdx)
		{
			// Go to right
			Node* right = it->m_saRight;
			if(right)
			{
				it = right;
			}
			else
			{
				parent = it;
				return &it->m_saRight;
			}
		}
		else if(idx < it->m_saIdx)
		{
			// Go to left
			Node* left = it->m_saLeft;
			if(left)
			{
				it = left;
			}
			else
			{
				parent = it;
				return &it->m_saLeft;
			}
		}
		else
		{
			// Equal
			parent = it->m_saParent;

			if(parent)
			{
				if(parent->m_saLeft == it)
				{
					return &parent->m_saLeft;
				}
				else
				{
					ANKI_ASSERT(parent->m_saRight == it);
					return &parent->m_saRight;
				}
			}
			else
			{
				return &m_bucket.m_elements[modIdx];
			}
		}
	}

	ANKI_ASSERT(!!"Shouldn't reach that");
	return nullptr;
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
const TNode* SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::tryGetNode(Index idx) const
{
	const Index modIdx = mod(idx);

	if(m_bucket.m_elements[modIdx] && mod(m_bucket.m_elements[modIdx]->m_saIdx) == modIdx)
	{
		// Walk the tree
		const Node* it = m_bucket.m_elements[modIdx];
		do
		{
			if(it->m_saIdx == idx)
			{
				return it;
			}
			else if(idx > it->m_saIdx)
			{
				it = it->m_saRight;
			}
			else
			{
				it = it->m_saLeft;
			}
		} while(it);
	}

	// Search for linear probing
	const Index endIdx = min(BUCKET_SIZE, modIdx + LINEAR_PROBING_SIZE);
	for(Index i = modIdx; i < endIdx; ++i)
	{
		const Node* const node = m_bucket.m_elements[i];
		if(node && node->m_saIdx == idx)
		{
			return node;
		}
	}

	return nullptr;
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
void SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::insertToTree(Node* const root, Node* node)
{
	ANKI_ASSERT(root && node);
	ANKI_ASSERT(root != node);
	ANKI_ASSERT(mod(root->m_saIdx) == mod(node->m_saIdx) && "Should belong to the same tree");

	const Index nodeIdx = node->m_saIdx;
	Node* it = root;
	Bool done = false;
	do
	{
		const Index idx = it->m_saIdx;
		if(nodeIdx > idx)
		{
			// Go to right
			Node* const right = it->m_saRight;
			if(right)
			{
				it = right;
			}
			else
			{
				node->m_saParent = it;
				it->m_saRight = node;
				done = true;
			}
		}
		else
		{
			// Go to left
			ANKI_ASSERT(idx != nodeIdx && "Can't do that");
			Node* const left = it->m_saLeft;
			if(left)
			{
				it = left;
			}
			else
			{
				node->m_saParent = it;
				it->m_saLeft = node;
				done = true;
			}
		}
	} while(!done);
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
TNode* SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::removeFromTree(Node* root, Node* del)
{
	ANKI_ASSERT(root && del);
	Node* const parent = del->m_saParent;
	Node* const left = del->m_saLeft;
	Node* const right = del->m_saRight;

	if(parent)
	{
		// If it has a parent then remove the connection to the parent and insert left and right like regular nodes

		if(parent->m_saLeft == del)
		{
			parent->m_saLeft = nullptr;
		}
		else
		{
			ANKI_ASSERT(parent->m_saRight == del);
			parent->m_saRight = nullptr;
		}

		if(left)
		{
			ANKI_ASSERT(left->m_saParent == del);
			left->m_saParent = nullptr;
			insertToTree(root, left);
		}

		if(right)
		{
			ANKI_ASSERT(right->m_saParent == del);
			right->m_saParent = nullptr;
			insertToTree(root, right);
		}
	}
	else
	{
		// It's the root node. Make arbitrarily the left root and add the right

		ANKI_ASSERT(del == root && "It must be the root");

		if(left)
		{
			left->m_saParent = nullptr;
			root = left;

			if(right)
			{
				right->m_saParent = nullptr;
				insertToTree(root, right);
			}
		}
		else
		{
			if(right)
			{
				right->m_saParent = nullptr;
			}

			root = right;
		}
	}

	return root;
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
void SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::remove(Iterator it)
{
	ANKI_ASSERT(it.m_node && it.m_array);
	ANKI_ASSERT(it.m_array == this);

	Node* const node = it.m_node;
	Node* const parent = node->m_saParent;
	Node* const left = node->m_saLeft;
	Node* const right = node->m_saRight;

	const Index modIdx = mod(node->m_saIdx);

	if(parent || left || right)
	{
		// In a tree, remove

		Node* root = m_bucket.m_elements[modIdx];
		ANKI_ASSERT(root);
		root = removeFromTree(root, node);

		m_bucket.m_elements[modIdx] = root;
	}
	else
	{
		// Not yet a tree, remove it from the bucket

		const Index endModIdx = min(modIdx + LINEAR_PROBING_SIZE, BUCKET_SIZE);
		Index i;
		for(i = modIdx; i < endModIdx; ++i)
		{
			if(m_bucket.m_elements[i] == node)
			{
				m_bucket.m_elements[i] = nullptr;
				break;
			}
		}

		ANKI_ASSERT(i < endModIdx && "Node not found");
	}
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
const TNode* SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::getNextNodeInternal(
	const Node* node) const
{
	ANKI_ASSERT(node);

	if(node->m_saLeft)
	{
		return node->m_saLeft;
	}

	if(node->m_saRight)
	{
		return node->m_saRight;
	}

	// Node without children but with a parent, move up the tree
	const Node* out = nullptr;
	if(node->m_saParent)
	{
		const Node* prevNode = node;
		out = node->m_saParent;
		do
		{
			if(out->m_saRight && out->m_saRight != prevNode)
			{
				return out->m_saRight;
			}

			prevNode = out;
			out = out->m_saParent;
		} while(out);

		// Apparently the node is the rightmost leaf
		node = prevNode;
		ANKI_ASSERT(node);
		ANKI_ASSERT(m_bucket.m_elements[mod(node->m_saIdx)] == node);
	}

	// Base of the tree, move to the next bucket element
	{
		const Index modIdx = mod(node->m_saIdx);

		// Find where the node is
		Index i = modIdx;
		do
		{
		} while(i < BUCKET_SIZE && node != m_bucket.m_elements[i++]);

		ANKI_ASSERT(i <= BUCKET_SIZE);

		// Now move to the next
		for(; i < BUCKET_SIZE; ++i)
		{
			if(m_bucket.m_elements[i])
			{
				return m_bucket.m_elements[i];
			}
		}
	}

	return nullptr;
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
void SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::validate() const
{
	PtrSize count = 0;

	for(Index i = 0; i < BUCKET_SIZE; ++i)
	{
		const Node* it = m_bucket.m_elements[i];
		if(it)
		{
			ANKI_ASSERT(it->m_saParent == nullptr);

			if(it->m_saLeft || it->m_saRight)
			{
				// It's a tree

				validateInternal(i, it, count);
			}
			else
			{
				// Check if it's linear probed

				const Index modIdx = mod(it->m_saIdx);
				if(modIdx != i)
				{
					ANKI_ASSERT(i > modIdx);
					ANKI_ASSERT(i - modIdx < LINEAR_PROBING_SIZE);
				}

				++count;
			}
		}
	}

	ANKI_ASSERT(count == m_elementCount);
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
void SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::validateInternal(
	Index modIdx, const Node* node, PtrSize& count) const
{
	ANKI_ASSERT(node);
	ANKI_ASSERT(mod(node->m_saIdx) == modIdx);

	++count;

	const Node* parent = node->m_saParent;
	(void)parent;
	const Node* left = node->m_saLeft;
	const Node* right = node->m_saRight;

	if(left)
	{
		ANKI_ASSERT(left->m_saParent == node);
		validateInternal(modIdx, left, count);
	}

	if(right)
	{
		ANKI_ASSERT(right->m_saParent == node);
		validateInternal(modIdx, right, count);
	}
}

template<typename T, typename TNode, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
TIndex SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>::findFirstModIdx() const
{
	for(Index i = 0; i < BUCKET_SIZE; i += 2)
	{
		if(m_bucket.m_elements[i])
		{
			return i;
		}
		else if(m_bucket.m_elements[i + 1])
		{
			return i + 1;
		}
	}

	return 0;
}

template<typename T, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
template<typename TAlloc>
void SparseArray<T, TIndex, TBucketSize, TLinearProbingSize>::destroyInternal(TAlloc& alloc, Node* const node)
{
	ANKI_ASSERT(node);

	Node* const left = node->m_saLeft;
	Node* const right = node->m_saRight;

	alloc.deleteInstance(node);

	if(left)
	{
		destroyInternal(alloc, left);
	}

	if(right)
	{
		destroyInternal(alloc, right);
	}
}

template<typename T, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
template<typename TAlloc>
void SparseArray<T, TIndex, TBucketSize, TLinearProbingSize>::destroy(TAlloc& alloc)
{
	for(Index i = 0; i < BUCKET_SIZE; ++i)
	{
		// Destroy the tree

		Node* root = m_bucket.m_elements[i];
		if(root)
		{
			destroyInternal(alloc, root);
			m_bucket.m_elements[i] = nullptr;
		}
	}

	m_elementCount = 0;
}

template<typename T, typename TIndex, PtrSize TBucketSize, PtrSize TLinearProbingSize>
template<typename TAlloc, typename... TArgs>
typename SparseArray<T, TIndex, TBucketSize, TLinearProbingSize>::Iterator
SparseArray<T, TIndex, TBucketSize, TLinearProbingSize>::setAt(TAlloc& alloc, Index idx, TArgs&&... args)
{
	Node* parent;
	Node** place = Base::findPlace(idx, parent);
	ANKI_ASSERT(place);

	if(*place)
	{
		// Node exists, recycle

		Node* const n = *place;
		n->m_saValue.~Value();
		::new(&n->m_saValue) Value(std::forward<TArgs>(args)...);
	}
	else
	{
		// Node doesn't exit,

		Node* newNode = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
		newNode->m_saIdx = idx;
		*place = newNode;
		newNode->m_saParent = parent;

		++Base::m_elementCount;
	}

	return Iterator(*place, this);
}

} // end namespace anki
