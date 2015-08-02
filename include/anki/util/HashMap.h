// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Ptr.h"

namespace anki {

/// @addtogroup util_private
/// @{

/// HashMap node. It's not a traditional bucket because it doesn't contain more
/// than one node.
template<typename T>
class HashMapNode
{
public:
	using Value = T;

	Value m_value;
	U64 m_hash;
	HashMapNode* m_parent = nullptr;
	HashMapNode* m_left = nullptr;
	HashMapNode* m_right = nullptr;

	template<typename... TArgs>
	HashMapNode(TArgs&&... args)
		: m_value(args...)
	{}
};

/// HashMap forward-only iterator.
template<typename TNodePointer, typename TValuePointer,
	typename TValueReference>
class HashMapIterator
{
public:
	TNodePointer m_node;

	/// Default constructor.
	HashMapIterator()
		: m_node(nullptr)
	{}

	/// Copy.
	HashMapIterator(const HashMapIterator& b)
		: m_node(b.m_node)
	{}

	/// Allow conversion from iterator to const iterator.
	template<typename YNodePointer, typename YValuePointer,
		typename YValueReference>
	HashMapIterator(const HashMapIterator<YNodePointer, YValuePointer,
		YValueReference>& b)
		: m_node(b.m_node)
	{}

	HashMapIterator(TNodePointer node)
		: m_node(node)
	{}

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

	HashMapIterator& operator++()
	{
		ANKI_ASSERT(m_node);
		TNodePointer node = m_node;

		if(node->m_left)
		{
			node = node->m_left;
		}
		else if(node->m_right)
		{
			node = node->m_right;
		}
		else
		{
			// Node without children
			TNodePointer prevNode = node;
			node = node->m_parent;
			while(node)
			{
				if(node->m_right && node->m_right != prevNode)
				{
					node = node->m_right;
					break;
				}
				prevNode = node;
				node = node->m_parent;
			}
		}

		m_node = node;
		return *this;
	}

	HashMapIterator operator++(int)
	{
		ANKI_ASSERT(m_node);
		HashMapIterator out = *this;
		++(*this);
		return out;
	}

	HashMapIterator operator+(U n) const
	{
		HashMapIterator it = *this;
		while(n-- != 0)
		{
			++it;
		}
		return it;
	}

	HashMapIterator& operator+=(U n)
	{
		while(n-- != 0)
		{
			++(*this);
		}
		return *this;
	}

	Bool operator==(const HashMapIterator& b) const
	{
		return m_node == b.m_node;
	}

	Bool operator!=(const HashMapIterator& b) const
	{
		return !(*this == b);
	}
};
/// @}

/// @addtogroup util_containers
/// @{

/// Hash map template.
template<typename T, typename THasher, typename TCompare,
	typename TNode = HashMapNode<T>>
class HashMap: public NonCopyable
{
public:
	using Value = T;
	using Node = TNode;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Pointer = Value*;
	using ConstPointer = const Value*;
	using Iterator = HashMapIterator<Node*, Pointer, Reference>;
	using ConstIterator =
		HashMapIterator<const Node*, ConstPointer, ConstReference>;

	/// Default constructor.
	HashMap()
		: m_root(nullptr)
	{}

	/// Move.
	HashMap(HashMap&& b)
		: HashMap()
	{
		move(b);
	}

	/// You need to manually destroy the map.
	/// @see HashMap::destroy
	~HashMap()
	{
		ANKI_ASSERT(m_root == nullptr && "Requires manual destruction");
	}

	/// Move.
	HashMap& operator=(HashMap&& b)
	{
		move(b);
		return *this;
	}

	/// Destroy the list.
	template<typename TAllocator>
	void destroy(TAllocator alloc);

	/// Get begin.
	Iterator getBegin()
	{
		return Iterator(m_root);
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return ConstIterator(m_root);
	}

	/// Get end.
	Iterator getEnd()
	{
		return Iterator();
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		return ConstIterator();
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

	/// Return true if map is empty.
	Bool isEmpty() const
	{
		return m_root == nullptr;
	}

	/// Copy an element in the map.
	template<typename TAllocator>
	void pushBack(TAllocator alloc, ConstReference x)
	{
		Node* node = alloc.template newInstance<Node>(x);
		node->m_hash = THasher()(node->m_value);
		pushBackNode(node);
	}

	/// Copy an element in the map.
	template<typename TAllocator>
	void pushBack(Pointer x)
	{
		ANKI_ASSERT(x);
		//static_assert(typeid(x) == );
		pushBackNode(x);
	}

	/// Construct an element inside the map.
	template<typename TAllocator, typename... TArgs>
	void emplaceBack(TAllocator alloc, TArgs&&... args)
	{
		Node* node = alloc.template newInstance<Node>(
			std::forward<TArgs>(args)...);
		node->m_hash = THasher()(node->m_value);
		pushBackNode(node);
	}

	/// Erase element.
	template<typename TAllocator>
	void erase(TAllocator alloc, Iterator it);

	/// Find item.
	Iterator find(const Value& a);

private:
	Node* m_root = nullptr;

	void move(HashMap& b)
	{
		ANKI_ASSERT(isEmpty() && "Cannot move before destroying");
		m_root = b.m_root;
		b.m_root = nullptr;
	}

	void pushBackNode(Node* node);

	Node* findInternal(Node* node, const Value& a, U64 aHash);

	template<typename TAllocator>
	void destroyInternal(TAllocator alloc, Node* node);
};
/// @}

} // end namespace anki

#include "anki/util/HashMap.inl.h"
