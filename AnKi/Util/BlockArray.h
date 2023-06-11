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
		return getAlignedRoundDown(ANKI_CACHE_LINE_SIZE, sizeof(T) * 64) / sizeof(T);
	}
};

/// BlockArray iterator.
template<typename TValuePointer, typename TValueReference, typename TBlockArrayPtr>
class BlockArrayIterator
{
public:
	/// Default constructor.
	BlockArrayIterator()
		: m_array(nullptr)
		, m_elementIdx(kMaxU32)
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(kMaxU32)
#endif
	{
	}

	/// Copy.
	BlockArrayIterator(const BlockArrayIterator& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(b.m_iteratorVer)
#endif
	{
	}

	/// Allow conversion from iterator to const iterator.
	template<typename YValuePointer, typename YValueReference, typename YBlockArrayPtr>
	BlockArrayIterator(const BlockArrayIterator<YValuePointer, YValueReference, YBlockArrayPtr>& b)
		: m_array(b.m_array)
		, m_elementIdx(b.m_elementIdx)
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(b.m_iteratorVer)
#endif
	{
	}

	BlockArrayIterator(TBlockArrayPtr arr, U32 idx
#if ANKI_EXTRA_CHECKS
					   ,
					   U32 ver
#endif
					   )
		: m_array(arr)
		, m_elementIdx(idx)
#if ANKI_EXTRA_CHECKS
		, m_iteratorVer(ver)
#endif
	{
		ANKI_ASSERT(arr);
	}

	BlockArrayIterator& operator=(const BlockArrayIterator& b)
	{
		m_array = b.m_array;
		m_elementIdx = b.m_elementIdx;
#if ANKI_EXTRA_CHECKS
		m_iteratorVer = b.m_iteratorVer;
#endif
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
		m_elementIdx = (m_elementIdx != kMaxU32) ? (m_elementIdx + 1) : kMaxU32;
		return *this;
	}

	BlockArrayIterator operator++(int)
	{
		check();
		BlockArrayIterator out = *this;
		++(*this);
		return out;
	}

	BlockArrayIterator operator+(U32 n) const
	{
		check();
		BlockArrayIterator out = *this;
		out.m_elementIdx = (out.m_elementIdx != kMaxU32) ? (out.m_elementIdx + n) : kMaxU32;
		return out;
	}

	BlockArrayIterator& operator+=(U32 n)
	{
		check();
		m_elementIdx = (m_elementIdx != kMaxU32) ? (m_elementIdx + n) : kMaxU32;
		return *this;
	}

	Bool operator==(const BlockArrayIterator& b) const
	{
		ANKI_ASSERT(m_array == b.m_array);
		ANKI_ASSERT(m_iteratorVer == b.m_iteratorVer);
		return m_elementIdx == b.m_elementIdx;
	}

	Bool operator!=(const BlockArrayIterator& b) const
	{
		return !(*this == b);
	}

private:
	TBlockArrayPtr m_array;
	U32 m_elementIdx;
#if ANKI_EXTRA_CHECKS
	U32 m_iteratorVer; ///< See BlockArray::m_iteratorVer.
#endif

	void check() const
	{
		ANKI_ASSERT(m_array);
	}
};

/// XXX
/// @tparam T
/// @tparam TMemoryPool
/// @tparam TConfig
template<typename T, typename TMemoryPool = SingletonMemoryPoolWrapper<DefaultMemoryPool>, typename TConfig = BlockArrayDefaultConfig<T>>
class BlockArray
{
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
		m_firstElement = b.m_firstElement;
		b.m_firstElement = kMaxU32;
		return *this;
	}

	ConstReference operator[](U32 idx) const
	{
		const U32 blockIdx = idx / kElementCountPerBlock;
		const U32 localIdx = idx % kElementCountPerBlock;
		ANKI_ASSERT(blockIdx < m_blockMetadatas.getSize());
		ANKI_ASSERT(m_blockStorages[blockIdx] != nullptr);
		ANKI_ASSERT(m_blockMetadatas[blockIdx].m_elementsInUseMask[localIdx].get() == true);
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
		return Iterator(this, m_firstElement
#if ANKI_EXTRA_CHECKS
						,
						m_iteratorVer
#endif
		);
	}

	/// Get begin.
	ConstIterator getBegin() const
	{
		return ConstIterator(this, m_firstElement
#if ANKI_EXTRA_CHECKS
							 ,
							 m_iteratorVer
#endif
		);
	}

	/// Get end.
	Iterator getEnd()
	{
		return Iterator(this, kMaxU32
#if ANKI_EXTRA_CHECKS
						,
						m_iteratorVer
#endif
		);
	}

	/// Get end.
	ConstIterator getEnd() const
	{
		return ConstIterator(this, kMaxU32
#if ANKI_EXTRA_CHECKS
							 ,
							 m_iteratorVer
#endif
		);
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
	void erase(ConstIterator at);

	TMemoryPool& getMemoryPool()
	{
		return m_blockStorages.getMemoryPool();
	}

private:
	class alignas(alignof(T)) BlockStorage
	{
	public:
		U8 m_storage[sizeof(T) * kElementCountPerBlock];
	};

	class BlockMetadata
	{
	public:
		BitSet<kElementCountPerBlock, U64> m_elementsInUseMask{false};
	};

	DynamicArray<BlockStorage*, TMemoryPool> m_blockStorages;
	DynamicArray<BlockMetadata, TMemoryPool> m_blockMetadatas;
	U32 m_elementCount = 0;
	U32 m_firstElement = kMaxU32;
#if ANKI_EXTRA_CHECKS
	U32 m_iteratorVer = 1;
#endif
};
/// @}

} // end namespace anki

#include <AnKi/Util/BlockArray.inl.h>
