// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Util/Thread.h>

namespace anki {

// Forward
template<typename TGpuSceneObject, U32 kId>
class GpuSceneArray;

/// @addtogroup scene
/// @{

/// @memberof GpuSceneArray
template<typename TGpuSceneObject, U32 kId>
class GpuSceneArrayAllocation
{
	friend class GpuSceneArray<TGpuSceneObject, kId>;

public:
	GpuSceneArrayAllocation() = default;

	GpuSceneArrayAllocation(const GpuSceneArrayAllocation& b) = delete;

	GpuSceneArrayAllocation(GpuSceneArrayAllocation&& b)
	{
		*this = std::move(b);
	}

	~GpuSceneArrayAllocation()
	{
		free();
	}

	GpuSceneArrayAllocation& operator=(const GpuSceneArrayAllocation&) = delete;

	GpuSceneArrayAllocation& operator=(GpuSceneArrayAllocation&& b)
	{
		free();
		m_index = b.m_index;
		b.invalidate();
		return *this;
	}

	explicit operator Bool() const
	{
		return isValid();
	}

	U32 getIndex() const
	{
		ANKI_ASSERT(isValid());
		return m_index;
	}

	Bool isValid() const
	{
		return m_index != kMaxU32;
	}

	/// Offset in the GPU scene.
	U32 getGpuSceneOffset() const;

	void uploadToGpuScene(const TGpuSceneObject& data) const
	{
		GpuSceneMicroPatcher::getSingleton().newCopy(getGpuSceneOffset(), data);
	}

	/// Allocate an element into the appropriate array. See GpuSceneArray::allocate()
	void allocate();

	/// Free the allocation. See GpuSceneArray::free()
	void free();

private:
	U32 m_index = kMaxU32;

	void invalidate()
	{
		m_index = kMaxU32;
	}
};

/// Contains a number an array that hold GPU scene objects of a specific type.
template<typename TGpuSceneObject, U32 kId>
class GpuSceneArray : public MakeSingleton<GpuSceneArray<TGpuSceneObject, kId>>
{
	template<typename>
	friend class MakeSingleton;

	friend class GpuSceneArrayAllocation<TGpuSceneObject, kId>;

public:
	using Allocation = GpuSceneArrayAllocation<TGpuSceneObject, kId>;

	/// @note Thread-safe
	PtrSize getGpuSceneOffsetOfArrayBase() const
	{
		return m_gpuSceneAllocation.getOffset();
	}

	/// @note Thread-safe
	PtrSize getBufferRange() const
	{
		return getElementCount() * getElementSize();
	}

	/// This count contains elements that may be innactive after a free. Frees in the middle of the array will not re-arrange other elements.
	/// @note Thread-safe
	U32 getElementCount() const
	{
		LockGuard lock(m_mtx);
		return (m_inUseIndicesCount) ? m_maxInUseIndex + 1 : 0;
	}

	constexpr static U32 getElementSize()
	{
		return sizeof(TGpuSceneObject);
	}

	/// @note Thread-safe
	BufferView getBufferView() const
	{
		return {&GpuSceneBuffer::getSingleton().getBuffer(), getGpuSceneOffsetOfArrayBase(), getBufferRange()};
	}

	/// @note Thread-safe
	BufferView getBufferViewSafe() const
	{
		PtrSize range = getBufferRange();
		if(range == 0)
		{
			// Just set something
			range = getElementSize();
		}

		return {&GpuSceneBuffer::getSingleton().getBuffer(), getGpuSceneOffsetOfArrayBase(), range};
	}

	/// Some bookeeping. Needs to be called once per frame.
	/// @note Thread-safe
	void flush()
	{
		flushInternal(true);
	}

private:
	using SubMask = BitSet<64, U64>;

	GpuSceneBufferAllocation m_gpuSceneAllocation;

	SceneDynamicArray<SubMask> m_inUseIndicesMask;

	U32 m_inUseIndicesCount = 0; ///< Doesn't count null elements.
	U32 m_maxInUseIndex = 0; ///< Counts null elements.

	SceneDynamicArray<U32> m_freedAllocations;

	mutable SpinLock m_mtx;

	GpuSceneArray(U32 maxArraySize);

	~GpuSceneArray();

	void validate() const;

	U32 getGpuSceneOffset(const Allocation& idx) const
	{
		ANKI_ASSERT(idx.isValid());
		return U32(getGpuSceneOffsetOfArrayBase() + sizeof(TGpuSceneObject) * idx.m_index);
	}

	/// @note Thread-safe
	Allocation allocate();

	/// The free will not resize the array if an element is somewhere in the middle. Resizing requires moves and will also invalidate pointers.
	/// Instead, the freed elements will stay as a dangling stale data inside their array. Nevertheless, the class will go and nullify those elements
	/// so they won't take part in visibility tests.
	/// @note Thread-safe
	void free(Allocation& alloc);

	void flushInternal(Bool nullifyElements);
};

template<typename TGpuSceneObject, U32 kId>
inline void GpuSceneArrayAllocation<TGpuSceneObject, kId>::free()
{
	GpuSceneArray<TGpuSceneObject, kId>::getSingleton().free(*this);
}

template<typename TGpuSceneObject, U32 kId>
inline void GpuSceneArrayAllocation<TGpuSceneObject, kId>::allocate()
{
	auto newAlloc = GpuSceneArray<TGpuSceneObject, kId>::getSingleton().allocate();
	*this = std::move(newAlloc);
}

template<typename TGpuSceneObject, U32 kId>
inline U32 GpuSceneArrayAllocation<TGpuSceneObject, kId>::getGpuSceneOffset() const
{
	return GpuSceneArray<TGpuSceneObject, kId>::getSingleton().getGpuSceneOffset(*this);
}

class GpuSceneArrays
{
public:
#define ANKI_CAT_TYPE(arrayName, gpuSceneType, id, cvarName) using arrayName = GpuSceneArray<gpuSceneType, id>;
#include <AnKi/Scene/GpuSceneArrays.def.h>
};
/// @}

} // end namespace anki

#include <AnKi/Scene/GpuSceneArray.inl.h>
