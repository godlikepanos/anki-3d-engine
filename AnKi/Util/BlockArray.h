// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/DynamicArray.h>

namespace anki {

/// @addtogroup util_containers
/// @{

/// Config options for a BlockArray.
/// @tparam T The type of the array.
template<typename T>
class BlockArrayDefaultConfig
{
public:
	static constexpr U32 getElementCountPerBlock()
	{
		return 64;
	}
};

/// BlockArray iterator.
template<typename TValuePointer, typename TValueReference, typename TBlockArrayPtr>
class BlockArrayIterator
{
	template<typename, typename, typename>
	friend class BlockArray;

	template<typename, typename, typename>
	friend class BlockArrayIterator;

public:
	/// Default constructor.
	BlockArrayIterator()
		: m_array(nullptr)
		, m_elementIdx(kMaxU32)
	{
	}

	/// Copy.
	BlockArrayIterator(const BlockArrayIterator& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
	{
	}

	/// Allow conversion from iterator to const iterator.
	template<typename YValuePointer, typename YValueReference, typename YBlockArrayPtr>
	BlockArrayIterator(const BlockArrayIterator<YValuePointer, YValueReference, YBlockArrayPtr>& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
	{
	}

	BlockArrayIterator(TBlockArrayPtr arr, U32 idx)
		: m_array(arr)
		, m_elementIdx(idx)
	{
	}

	BlockArrayIterator& operator=(const BlockArrayIterator& b)
	{
		m_array = b.m_array;
		m_elementIdx = b.m_elementIdx;
		return *this;
	}

	TValueReference operator*() const
	{
		check();
		return (*m_array)[m_elementIdx];
	}

	TValuePointer operator->() const
	{
		check();
		return &(*m_array)[m_elementIdx];
	}

	BlockArrayIterator& operator++()
	{
		check();
		m_elementIdx = m_array->getNextElementIndex(m_elementIdx);
		return *this;
	}

	BlockArrayIterator operator++(int)
	{
		check();
		BlockArrayIterator out = *this;
		++(*this);
		return out;
	}

	Bool operator==(const BlockArrayIterator& b) const
	{
		ANKI_ASSERT(m_array == b.m_array);
		return m_elementIdx == b.m_elementIdx;
	}

	Bool operator!=(const BlockArrayIterator& b) const
	{
		return !(*this == b);
	}

	/// Returns the imaginary index inside the BlockArray.
	U32 getArrayIndex() const
	{
		check();
		return m_elementIdx;
	}

private:
	TBlockArrayPtr m_array;
	U32 m_elementIdx;

	void check() const
	{
		ANKI_ASSERT(m_array);
		ANKI_ASSERT(m_array->indexExists(m_elementIdx));
	}
};

/// It's a type of dynamic array that unlike DynamicArray doesn't move elements around when it shrinks or grows the storage.
template<typename T, typename TMemoryPool = SingletonMemoryPoolWrapper<DefaultMemoryPool>, typename TConfig = BlockArrayDefaultConfig<T>>
class BlockArray
{
	template<typename, typename, typename>
	friend class BlockArrayIterator;

public:
	// Typedefs
	using Config = TConfig;
	using Value = T;
	using Iterator = BlockArrayIterator<T*, T&, BlockArray*>;
	using ConstIterator = BlockArrayIterator<const T*, const T&, const BlockArray*>;
	using Reference = Value&;
	using ConstReference = const Value&;

	static constexpr U32 kElementCountPerBlock = TConfig::getElementCountPerBlock();

	BlockArray(const TMemoryPool& pool = TMemoryPool())
		: m_blockStorages(pool)
		, m_blockMetadatas(pool)
	{
	}

	/// Copy.
	BlockArray(const BlockArray& b)
	{
		*this = b;
	}

	/// Move.
	BlockArray(BlockArray&& b)
	{
		*this = std::move(b);
	}

	/// Destroy.
	~BlockArray()
	{
		destroy();
	}

	/// Copy.
	BlockArray& operator=(const BlockArray& b);

	/// Move operator.
	BlockArray& operator=(BlockArray&& b)
	{
		destroy();
		m_blockStorages = std::move(b.m_blockStorages);
		m_blockMetadatas = std::move(b.m_blockMetadatas);
		m_elementCount = b.m_elementCount;
		b.m_elementCount = 0;
		m_firstIndex = b.m_firstIndex;
		b.m_firstIndex = 0;
		m_endIndex = b.m_endIndex;
		b.m_endIndex = 0;
		return *this;
	}

	ConstReference operator[](U32 idx) const
	{
		const U32 blockIdx = idx / kElementCountPerBlock;
		const U32 localIdx = idx % kElementCountPerBlock;
		ANKI_ASSERT(blockIdx < m_blockMetadatas.getSize());
		ANKI_ASSERT(m_blockStorages[blockIdx] != nullptr);
		ANKI_ASSERT(m_blockMetadatas[blockIdx].m_elementsInUseMask.get(localIdx) == true);
		return *reinterpret_cast<Value*>(&m_blockStorages[blockIdx]->m_storage[localIdx * sizeof(Value)]);
	}

	Reference operator[](U32 idx)
	{
		const auto& constSelf = *this;
		return const_cast<Reference>(constSelf[idx]);
	}

	/// Get begin.
	Iterator getBegin()
	{
		return Iterator(this, m_firstIndex);
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return ConstIterator(this, m_firstIndex);
	}

	/// Get end.
	Iterator getEnd()
	{
		return Iterator(this, m_endIndex);
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		return ConstIterator(this, m_endIndex);
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

	U32 getSize() const
	{
		return m_elementCount;
	}

	Bool isEmpty() const
	{
		return m_elementCount == 0;
	}

	/// Destroy the array and free its elements.
	void destroy();

	/// Emplace somewhere in some block.
	template<typename... TArgs>
	Iterator emplace(TArgs&&... args);

	/// Removes one element.
	/// @param at Points to the position of the element to remove.
	void erase(Iterator idx);

	/// Removes one element.
	/// @param at Points to the position of the element to remove.
	void erase(U32 index)
	{
		erase(indexToIterator(index));
	}

	Iterator indexToIterator(U32 idx)
	{
		ANKI_ASSERT(indexExists(idx));
		return Iterator(this, idx);
	}

	ConstIterator indexToIterator(U32 idx) const
	{
		ANKI_ASSERT(indexExists(idx));
		return ConstIterator(this, idx);
	}

	Bool indexExists(U32 idx) const
	{
		const U32 localIdx = idx % kElementCountPerBlock;
		const U32 blockIdx = idx / kElementCountPerBlock;
		return blockIdx < m_blockMetadatas.getSize() && m_blockMetadatas[blockIdx].m_elementsInUseMask.get(localIdx) == true;
	}

	TMemoryPool& getMemoryPool()
	{
		return m_blockStorages.getMemoryPool();
	}

	void validate() const;

private:
	class alignas(alignof(T)) BlockStorage
	{
	public:
		U8 m_storage[sizeof(T) * kElementCountPerBlock];
	};

	using Mask = BitSet<kElementCountPerBlock, U64>;

	class BlockMetadata
	{
	public:
		Mask m_elementsInUseMask{false};

		BlockMetadata() = default;

		BlockMetadata(Bool set)
			: m_elementsInUseMask(set)
		{
		}
	};

	DynamicArray<BlockStorage*, TMemoryPool> m_blockStorages;
	DynamicArray<BlockMetadata, TMemoryPool> m_blockMetadatas;
	U32 m_elementCount = 0;
	U32 m_firstIndex = 0;
	U32 m_endIndex = 0; ///< The index after the last.

	U32 getFirstElementIndex() const
	{
		for(U32 blockIdx = 0; blockIdx < m_blockMetadatas.getSize(); ++blockIdx)
		{
			U32 localIdx;
			if((localIdx = m_blockMetadatas[blockIdx].m_elementsInUseMask.getLeastSignificantBit()) != kMaxU32)
			{
				return localIdx + blockIdx * kElementCountPerBlock;
			}
		}

		ANKI_ASSERT(0);
		return 0;
	}

	U32 getLastElementIndex() const
	{
		U32 blockIdx = m_blockMetadatas.getSize();
		while(blockIdx--)
		{
			U32 localIdx;
			if((localIdx = m_blockMetadatas[blockIdx].m_elementsInUseMask.getMostSignificantBit()) != kMaxU32)
			{
				return localIdx + blockIdx * kElementCountPerBlock;
			}
		}

		ANKI_ASSERT(0);
		return 0;
	}

	U32 getNextElementIndex(U32 crnt) const;
};
/// @}

} // end namespace anki

#include <AnKi/Util/BlockArray.inl.h>
