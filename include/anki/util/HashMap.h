// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_HASH_MAP_H
#define ANKI_UTIL_HASH_MAP_H

#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"

/// @addtogroup util_private
/// @{

/// HashMap node.
template<typename T>
class HashMapNode
{
public:
	using Value = T;

	Value m_value;
	U64 m_hash;
	HashMapNode* m_prev = nullptr;
	HashMapNode* m_next = nullptr;

	template<typename... TArgs>
	HashMapNode(TArgs&&... args)
	:	m_value(std::forward<TArgs>(args)...)
	{}
};

/// HashMap bidirectional iterator.
template<typename TNodePointer, typename TValuePointer, 
	typename TValueReference, typename THashMapPointer>
class HashMapIterator
{
public:
	TNodePointer m_node = nullptr;
	THashMapPointer m_map = nullptr; ///< Used to go back from the end

	HashMapIterator() = default;

	HashMapIterator(const HashMapIterator& b)
	:	m_node(b.m_node),
		m_map(b.m_map)
	{}

	/// Allow conversion from iterator to const iterator.
	template<typename YNodePointer, typename YValuePointer, 
		typename YValueReference, typename YHashMap>
	HashMapIterator(const HashMapIterator<YNodePointer, 
		YValuePointer, YValueReference, YHashMap>& b)
	:	m_node(b.m_node),
		m_map(b.m_map)
	{}

	HashMapIterator(TNodePointer node, THashMapPointer list)
	:	m_node(node),
		m_map(list)
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

	HashMapIterator& operator++()
	{
		ANKI_ASSERT(m_node);
		m_node = m_node->m_next;
		return *this;
	}

	HashMapIterator operator++(int)
	{
		ANKI_ASSERT(m_node);
		HashMapIterator out = *this;
		++(*this);
		return out;
	}

	HashMapIterator& operator--();

	HashMapIterator operator--(int)
	{
		ANKI_ASSERT(m_node);
		HashMapIterator out = *this;
		--(*this);
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

	HashMapIterator operator-(U n) const
	{
		HashMapIterator it = *this;
		while(n-- != 0)
		{
			--it;
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

	HashMapIterator& operator-=(U n)
	{
		while(n-- != 0)
		{
			--(*this);
		}
		return *this;
	}

	Bool operator==(const HashMapIterator& b) const
	{
		ANKI_ASSERT(m_list == b.m_list 
			&& "Comparing iterators from different lists");
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
template<typename T>
class HashMap
{
public:
	using Value = T;
	using Node = HashMapNode<Value>;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Pointer = Value*;
	using ConstPointer = const Value*;

	HashMap() = default;

	/// Move.
	HashMap(HashMap&& b)
	:	HashMap()
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

	/// Return true if map is empty.
	Bool isEmpty() const
	{
		return m_root == nullptr;
	}

private:
	Node* m_root = nullptr;

	void move(HashMap& b)
	{
		ANKI_ASSERT(isEmpty() && "Cannot move before destroying");
		m_root = b.m_root;
		b.m_root = nullptr;
	}
};
/// @}

#endif

