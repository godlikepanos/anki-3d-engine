// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/Functions.h>
#include <anki/util/NonCopyable.h>

namespace anki
{

// Forward
template<typename, typename, typename, typename>
class HashMap;

template<typename, typename, typename, typename>
class IntrusiveHashMap;

/// @addtogroup util_containers
/// @{

namespace detail
{

/// HashMap node. It's not a traditional bucket because it doesn't contain more than one values.
/// @internal
template<typename TValue>
class HashMapNode
{
public:
	U64 m_hash = 0;
	HashMapNode* m_left = nullptr;
	HashMapNode* m_right = nullptr;
	TValue m_value;
	HashMapNode* m_parent = nullptr; ///< Used for iterating.

	template<typename... TArgs>
	HashMapNode(TArgs&&... args)
		: m_value(std::forward<TArgs>(args)...)
	{
	}

	TValue& getHashMapNodeValue()
	{
		return m_value;
	}

	const TValue& getHashMapNodeValue() const
	{
		return m_value;
	}
};

/// HashMap forward-only iterator.
/// @internal
template<typename TNodePointer, typename TValuePointer, typename TValueReference>
class HashMapIterator
{
	template<typename, typename, typename, typename>
	friend class anki::HashMap;

	template<typename, typename, typename, typename>
	friend class anki::IntrusiveHashMap;

public:
	/// Default constructor.
	HashMapIterator()
		: m_node(nullptr)
	{
	}

	/// Copy.
	HashMapIterator(const HashMapIterator& b)
		: m_node(b.m_node)
	{
	}

	/// Allow conversion from iterator to const iterator.
	template<typename YNodePointer, typename YValuePointer, typename YValueReference>
	HashMapIterator(const HashMapIterator<YNodePointer, YValuePointer, YValueReference>& b)
		: m_node(b.m_node)
	{
	}

	HashMapIterator(TNodePointer node)
		: m_node(node)
	{
	}

	TValueReference operator*() const
	{
		ANKI_ASSERT(m_node);
		return m_node->getHashMapNodeValue();
	}

	TValuePointer operator->() const
	{
		ANKI_ASSERT(m_node);
		return &m_node->getHashMapNodeValue();
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

private:
	TNodePointer m_node;
};

/// Hash map base.
/// @tparam TKey The key of the map.
/// @tparam TValue The value of the map.
/// @tparam THasher Functor to hash type of TKey.
/// @tparam TCompare Functor to compare TKey.
/// @internal
template<typename TKey, typename TValue, typename THasher, typename TCompare, typename TNode>
class HashMapBase : public NonCopyable
{
public:
	using Key = TKey;
	using Value = TValue;
	using Reference = Value&;
	using ConstReference = const Value&;
	using Pointer = Value*;
	using ConstPointer = const Value*;
	using Iterator = HashMapIterator<TNode*, Pointer, Reference>;
	using ConstIterator = HashMapIterator<const TNode*, ConstPointer, ConstReference>;

	/// Default constructor.
	HashMapBase()
		: m_root(nullptr)
	{
	}

	~HashMapBase() = default;

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

	/// Find item.
	Iterator find(const Key& key);

	/// Find item.
	ConstIterator find(const Key& key) const;

protected:
	/// @privatesection
	TNode* m_root = nullptr;

	void move(HashMapBase& b)
	{
		m_root = b.m_root;
		b.m_root = nullptr;
	}

	/// Add a node in the tree.
	void insertNode(TNode* node);

	/// Remove a node from the tree.
	void removeNode(TNode* node);
};

} // end namespace detail

/// Default hash key compare.
template<typename TKey>
class DefaultHashKeyCompare
{
public:
	Bool operator()(const TKey& a, const TKey& b) const
	{
		return a == b;
	}
};

/// Default hasher.
template<typename TKey>
class DefaultHasher
{
public:
	U64 operator()(const TKey& a) const
	{
		return a.genHash();
	}
};

/// Specialization for U64 keys.
template<>
class DefaultHasher<U64>
{
public:
	U64 operator()(const U64 a) const
	{
		return a;
	}
};

/// Hash map template.
template<typename TKey,
	typename TValue,
	typename THasher = DefaultHasher<TKey>,
	typename TCompare = DefaultHashKeyCompare<TKey>>
class HashMap : public detail::HashMapBase<TKey, TValue, THasher, TCompare, detail::HashMapNode<TValue>>
{
private:
	using Base = detail::HashMapBase<TKey, TValue, THasher, TCompare, detail::HashMapNode<TValue>>;
	using Node = detail::HashMapNode<TValue>;

public:
	/// Default constructor.
	HashMap()
		: Base()
	{
	}

	/// Move.
	HashMap(HashMap&& b)
		: Base()
	{
		Base::move(b);
	}

	/// You need to manually destroy the map.
	/// @see HashMap::destroy
	~HashMap()
	{
		ANKI_ASSERT(Base::m_root == nullptr && "Requires manual destruction");
	}

	/// Move.
	HashMap& operator=(HashMap&& b)
	{
		Base::move(b);
		return *this;
	}

	/// Destroy the list.
	template<typename TAllocator>
	void destroy(TAllocator alloc);

	/// Copy an element in the map.
	template<typename TAllocator>
	void pushBack(TAllocator alloc, const TKey& key, const TValue& x)
	{
		Node* node = alloc.template newInstance<Node>(x);
		node->m_hash = THasher()(key);
		Base::insertNode(node);
	}

	/// Construct an element inside the map.
	template<typename TAllocator, typename... TArgs>
	void emplaceBack(TAllocator alloc, const TKey& key, TArgs&&... args)
	{
		Node* node = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
		node->m_hash = THasher()(key);
		Base::insertNode(node);
	}

	/// Erase element.
	template<typename TAllocator>
	void erase(TAllocator alloc, typename Base::Iterator it)
	{
		Node* del = it.m_node;
		Base::removeNode(del);
		alloc.deleteInstance(del);
	}

private:
	template<typename TAllocator>
	void destroyInternal(TAllocator alloc, Node* node);
};

/// The classes that will use the IntrusiveHashMap need to inherit from this one.
template<typename TClass>
class IntrusiveHashMapEnabled : public NonCopyable
{
	template<typename TKey, typename TValue, typename THasher, typename TCompare, typename TNode>
	friend class detail::HashMapBase;

	template<typename TNodePointer, typename TValuePointer, typename TValueReference>
	friend class detail::HashMapIterator;

	template<typename TKey, typename TValue, typename THasher, typename TCompare>
	friend class IntrusiveHashMap;

	friend TClass;

public:
	/// Default constructor.
	IntrusiveHashMapEnabled() = default;

	/// Move.
	IntrusiveHashMapEnabled(IntrusiveHashMapEnabled&& b)
		: m_hash(b.m_hash)
		, m_left(b.m_left)
		, m_right(b.m_right)
		, m_parent(b.m_parent)
	{
		b.m_hash = 0;
		b.m_left = b.m_right = b.m_parent = nullptr;
	}

	/// Move.
	IntrusiveHashMapEnabled& operator=(IntrusiveHashMapEnabled&& b)
	{
		ANKI_ASSERT(m_hash == 0 && m_left == m_right && m_right == m_parent && m_parent == nullptr);
		m_hash = b.m_hash;
		m_left = b.m_left;
		m_right = b.m_right;
		m_parent = b.m_parent;
		b.m_hash = 0;
		b.m_left = b.m_right = b.m_parent = nullptr;
		return *this;
	}

private:
	U64 m_hash = 0;
	TClass* m_left = nullptr;
	TClass* m_right = nullptr;
	TClass* m_parent = nullptr; ///< Used for iterating.

	TClass& getHashMapNodeValue()
	{
		return *static_cast<TClass*>(this);
	}

	const TClass& getHashMapNodeValue() const
	{
		return *static_cast<const TClass*>(this);
	}
};

/// Hash map that doesn't perform any allocations. To work the TValue nodes will have to inherit from
/// IntrusiveHashMapEnabled.
template<typename TKey,
	typename TValue,
	typename THasher = DefaultHasher<TKey>,
	typename TCompare = DefaultHashKeyCompare<TKey>>
class IntrusiveHashMap : public detail::HashMapBase<TKey, TValue, THasher, TCompare, TValue>
{
private:
	using Base = detail::HashMapBase<TKey, TValue, THasher, TCompare, TValue>;
	using Node = TValue;

public:
	/// Default constructor.
	IntrusiveHashMap()
		: Base()
	{
	}

	/// Move.
	IntrusiveHashMap(IntrusiveHashMap&& b)
		: Base()
	{
		Base::move(b);
	}

	~IntrusiveHashMap() = default;

	/// Move.
	IntrusiveHashMap& operator=(IntrusiveHashMap&& b)
	{
		Base::move(b);
		return *this;
	}

	/// Add an element to the map.
	void pushBack(const TKey& key, TValue* x)
	{
		ANKI_ASSERT(x);
		IntrusiveHashMapEnabled<TValue>* e = static_cast<IntrusiveHashMapEnabled<TValue>*>(x);
		e->m_hash = THasher()(key);
		Base::insertNode(x);
	}

	/// Erase element.
	void erase(typename Base::Iterator it)
	{
		Node* del = it.m_node;
		Base::removeNode(del);
	}
};
/// @}

} // end namespace anki

#include <anki/util/HashMap.inl.h>
