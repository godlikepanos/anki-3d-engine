// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Assert.h>
#include <anki/util/Array.h>
#include <anki/util/Allocator.h>
#include <utility>
#include <ctime>

namespace anki
{

/// @addtogroup util_containers
/// @{

/// A single bucket of SparseArray elements. It's its own class in case we allow multiple buckets in the future.
/// @internal
template<typename TNode, PtrSize TBucketSize>
class SparseArrayBucket
{
public:
	Array<TNode*, TBucketSize> m_elements;
};

/// SparseArray node.
/// @internal
template<typename T>
class SparseArrayNode
{
public:
	SparseArrayNode* m_saLeft = nullptr;
	SparseArrayNode* m_saRight = nullptr;
	SparseArrayNode* m_saParent = nullptr; ///< Used for iterating.
	PtrSize m_saIdx = 0; ///< The index in the sparse array.
	T m_saValue;

	template<typename... TArgs>
	SparseArrayNode(TArgs&&... args)
		: m_saValue(std::forward<TArgs>(args)...)
	{
	}

	T& getSaValue()
	{
		return m_saValue;
	}

	const T& getSaValue() const
	{
		return m_saValue;
	}
};

/// Sparse array iterator.
template<typename TNodePointer, typename TValuePointer, typename TValueReference, typename TSparseArrayBasePtr>
class SparseArrayIterator
{
	template<typename, typename, typename, PtrSize, PtrSize>
	friend class SparseArrayBase;

	template<typename, typename, PtrSize, PtrSize>
	friend class SparseArray;

public:
	/// Default constructor.
	SparseArrayIterator()
		: m_node(nullptr)
		, m_array(nullptr)
	{
	}

	/// Copy.
	SparseArrayIterator(const SparseArrayIterator& b)
		: m_node(b.m_node)
		, m_array(b.m_array)
	{
	}

	/// Allow conversion from iterator to const iterator.
	template<typename YNodePointer, typename YValuePointer, typename YValueReference, typename YSparseArrayBasePtr>
	SparseArrayIterator(const SparseArrayIterator<YNodePointer, YValuePointer, YValueReference, YSparseArrayBasePtr>& b)
		: m_node(b.m_node)
		, m_array(b.m_array)
	{
	}

	SparseArrayIterator(TNodePointer node, TSparseArrayBasePtr arr)
		: m_node(node)
		, m_array(arr)
	{
		ANKI_ASSERT(arr);
	}

	TValueReference operator*() const
	{
		ANKI_ASSERT(m_node && m_array);
		return m_node->getSaValue();
	}

	TValuePointer operator->() const
	{
		ANKI_ASSERT(m_node && m_array);
		return &m_node->getSaValue();
	}

	SparseArrayIterator& operator++()
	{
		ANKI_ASSERT(m_node && m_array);
		m_node = m_array->getNextNode(m_node);
		return *this;
	}

	SparseArrayIterator operator++(int)
	{
		ANKI_ASSERT(m_node && m_array);
		SparseArrayIterator out = *this;
		++(*this);
		return out;
	}

	SparseArrayIterator operator+(U n) const
	{
		SparseArrayIterator it = *this;
		while(n-- != 0)
		{
			++it;
		}
		return it;
	}

	SparseArrayIterator& operator+=(U n)
	{
		while(n-- != 0)
		{
			++(*this);
		}
		return *this;
	}

	Bool operator==(const SparseArrayIterator& b) const
	{
		return m_node == b.m_node;
	}

	Bool operator!=(const SparseArrayIterator& b) const
	{
		return !(*this == b);
	}

	operator Bool() const
	{
		return m_node != nullptr;
	}

private:
	TNodePointer m_node;
	TSparseArrayBasePtr m_array;
};

/// Sparse array base class.
/// @internal
template<typename T, typename TNode, typename TIndex = U32, PtrSize TBucketSize = 128, PtrSize TLinearProbingSize = 4>
class SparseArrayBase
{
	template<typename, typename, typename, typename>
	friend class SparseArrayIterator;

public:
	// Typedefs
	using Self = SparseArrayBase<T, TNode, TIndex, TBucketSize, TLinearProbingSize>;
	using Value = T;
	using Node = TNode;
	using Index = TIndex;
	using Iterator = SparseArrayIterator<TNode*, T*, T&, Self*>;
	using ConstIterator = SparseArrayIterator<const TNode*, const T*, const T&, const Self*>;

	// Some constants
	static constexpr PtrSize BUCKET_SIZE = TBucketSize;
	static constexpr PtrSize LINEAR_PROBING_SIZE = TLinearProbingSize;

	/// Get begin.
	Iterator getBegin()
	{
		return Iterator(m_bucket.m_elements[m_firstElementModIdx], this);
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return ConstIterator(m_bucket.m_elements[m_firstElementModIdx], this);
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

	/// Get the number of elements in the array.
	PtrSize getSize() const
	{
		return m_elementCount;
	}

	/// Return true if it's empty and false otherwise.
	Bool isEmpty() const
	{
		return m_elementCount != 0;
	}

protected:
	SparseArrayBucket<Node, BUCKET_SIZE> m_bucket;
	Index m_firstElementModIdx = ~Index(0); ///< To start iterating without searching. Points to m_bucket.
	U32 m_elementCount = 0;

	/// Default constructor.
	SparseArrayBase()
	{
		ANKI_ASSERT(isPowerOfTwo(BUCKET_SIZE));
		zeroMemory(m_bucket);
	}

	/// Non-copyable.
	SparseArrayBase(const SparseArrayBase&) = delete;

	/// Move constructor.
	SparseArrayBase(SparseArrayBase&& b)
	{
		*this = std::move(b);
	}

	/// Destroy.
	~SparseArrayBase()
	{
#if ANKI_EXTRA_CHECKS
		zeroMemory(m_bucket);
#endif
	}

	/// Non-copyable.
	SparseArrayBase& operator=(const SparseArrayBase&) = delete;

	/// Move operator.
	SparseArrayBase& operator=(SparseArrayBase&& b)
	{
		if(b.m_elementCount)
		{
			memcpy(&m_bucket[0], &b.m_bucket[0], sizeof(m_bucket));
			m_elementCount = b.m_elementCount;
			zeroMemory(b.m_bucket);
			b.m_elementCount = 0;
		}
		else
		{
			zeroMemory(m_bucket);
			m_elementCount = 0;
		}

		return *this;
	}

	/// Find a place for a node in the array.
	Node** findPlace(Index idx);

	/// Remove a node.
	void remove(Iterator it);

	/// Try get an node.
	const Node* tryGetNode(Index idx) const;

	/// For iterating.
	const Node* getNextNode(const Node* const node) const;

	/// For iterating.
	Node* getNextNode(const Node* node)
	{
		const Node* out = getNextNode(node);
		return const_cast<Node*>(out);
	}

private:
	static Index mod(const Index idx)
	{
		return idx & (BUCKET_SIZE - 1);
	}

	static void insertToTree(Node* const root, Node* node);

	/// Remove a node from the tree.
	/// @return The new root node.
	static Node* removeFromTree(Node* root, Node* del);
};

/// Sparse array.
/// @tparam T The type of the valut it will hold.
/// @tparam TIndex The type of the index. It should be U32 or U64.
/// @tparam TBucketeSize The number of the preallocated size.
/// @tparam TLinearProbingSize The number of positions that will be linearly probed when inserting.
template<typename T, typename TIndex = U32, PtrSize TBucketSize = 128, PtrSize TLinearProbingSize = 4>
class SparseArray : public SparseArrayBase<T, SparseArrayNode<T>, TIndex, TBucketSize, TLinearProbingSize>
{
public:
	using Base = SparseArrayBase<T, SparseArrayNode<T>, TIndex, TBucketSize, TLinearProbingSize>;
	using Node = typename Base::Node;
	using Value = typename Base::Value;
	using Index = typename Base::Index;
	using Iterator = typename Base::Iterator;
	using ConstIterator = typename Base::ConstIterator;

	using Base::BUCKET_SIZE;
	using Base::LINEAR_PROBING_SIZE;

	SparseArray()
	{
	}

	/// Non-copyable.
	SparseArray(const SparseArray&) = delete;

	/// Move constructor.
	SparseArray(SparseArray&& b)
	{
		*this = std::move(b);
	}

	/// Destroy. It will do nothing.
	~SparseArray()
	{
		ANKI_ASSERT(m_elementCount == 0 && "Forgot to call destroy");
	}

	/// Non-copyable.
	SparseArray& operator=(const SparseArray&) = delete;

	/// Move operator.
	SparseArray& operator=(SparseArray&& b)
	{
		ANKI_ASSERT(m_elementCount == 0 && "Forgot to destroy");
		static_cast<Base&>(*this) = std::move(static_cast<Base&>(b));
		return *this;
	}

	/// Destroy the array and free its elements.
	template<typename TAlloc>
	void destroy(TAlloc& alloc);

	/// Set a value to an index.
	template<typename TAlloc, typename... TArgs>
	Iterator setAt(TAlloc& alloc, Index idx, TArgs&&... args)
	{
		Node* newNode = alloc.template newInstance<Node>(std::forward<TArgs>(args)...);
		newNode->m_saIdx = idx;
		Node** place = Base::findPlace(idx);
		ANKI_ASSERT(place);
		if(*place)
		{
			alloc.deleteInstance(*place);
		}
		else
		{
			++Base::m_elementCount;
		}
		*place = newNode;
		return Iterator(newNode, this);
	}

	/// Get an iterator.
	Iterator getAt(Index idx)
	{
		const Node* node = Base::tryGetNode(idx);
		return Iterator(const_cast<Node*>(node), this);
	}

	/// Get an iterator.
	ConstIterator getAt(Index idx) const
	{
		const Node* node = Base::tryGetNode(idx);
		return ConstIterator(node, this);
	}

	/// Remove an element.
	template<typename TAlloc>
	void erase(TAlloc& alloc, Iterator it)
	{
		Base::remove(it);
		alloc.deleteInstance(it.m_node);
		ANKI_ASSERT(Base::m_elementCount > 0);
		--m_elementCount;
	}

private:
	using Base::m_elementCount;
	using Base::m_bucket;

	template<typename TAlloc>
	void destroyInternal(TAlloc& alloc, Node* const root);
};
/// @}

} // end namespace anki

#include <anki/util/SparseArray.inl.h>
