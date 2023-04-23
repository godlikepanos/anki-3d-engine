// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Some of the GPU scene structures are stored in structured buffers
enum class GpuSceneContiguousArrayType : U8
{
	kTransformPairs,
	kMeshLods,
	kParticleEmitters,
	kPointLights,
	kSpotLights,
	kReflectionProbes,
	kGlobalIlluminationProbes,
	kDecals,
	kFogDensityVolumes,
	kRenderables,

	kRenderableBoundingVolumesGBuffer,
	kRenderableBoundingVolumesForward,
	kRenderableBoundingVolumesDepth,

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GpuSceneContiguousArrayType)

class GpuSceneContiguousArrayIndex
{
	friend class AllGpuSceneContiguousArrays;

public:
	GpuSceneContiguousArrayIndex() = default;

	GpuSceneContiguousArrayIndex(const GpuSceneContiguousArrayIndex& b) = delete;

	GpuSceneContiguousArrayIndex(GpuSceneContiguousArrayIndex&& b)
	{
		*this = std::move(b);
	}

	~GpuSceneContiguousArrayIndex();

	GpuSceneContiguousArrayIndex& operator=(const GpuSceneContiguousArrayIndex&) = delete;

	GpuSceneContiguousArrayIndex& operator=(GpuSceneContiguousArrayIndex&& b)
	{
		ANKI_ASSERT(!isValid());
		m_index = b.m_index;
		m_type = b.m_type;
		b.invalidate();
		return *this;
	}

	U32 get() const
	{
		ANKI_ASSERT(m_index != kMaxU32);
		return m_index;
	}

	Bool isValid() const
	{
		return m_index != kMaxU32;
	}

	U32 getOffsetInGpuScene() const;

private:
	U32 m_index = kMaxU32;
	GpuSceneContiguousArrayType m_type = GpuSceneContiguousArrayType::kCount;

	void invalidate()
	{
		m_index = kMaxU32;
		m_type = GpuSceneContiguousArrayType::kCount;
	}
};

/// Contains a number of contiguous array allocators for various GPU scene contiguous objects.
class AllGpuSceneContiguousArrays : public MakeSingleton<AllGpuSceneContiguousArrays>
{
	template<typename>
	friend class MakeSingleton;

public:
	/// @note Thread-safe against allocate(), deferredFree() and endFrame()
	GpuSceneContiguousArrayIndex allocate(GpuSceneContiguousArrayType type);

	/// @note It's not thread-safe
	PtrSize getArrayBase(GpuSceneContiguousArrayType type) const
	{
		return m_allocs[type].getArrayBase();
	}

	/// @note It's not thread-safe
	U32 getElementCount(GpuSceneContiguousArrayType type) const
	{
		return m_allocs[type].getElementCount();
	}

	/// @note It's not thread-safe
	U32 getElementOffsetInGpuScene(const GpuSceneContiguousArrayIndex& idx) const
	{
		ANKI_ASSERT(idx.isValid());
		return U32(getArrayBase(idx.m_type) + m_componentCount[idx.m_type] * m_componentSize[idx.m_type] * idx.m_index);
	}

	constexpr static U32 getElementSize(GpuSceneContiguousArrayType type)
	{
		return m_componentSize[type] * m_componentCount[type];
	}

	/// @note Thread-safe against allocate(), deferredFree() and endFrame()
	void deferredFree(GpuSceneContiguousArrayIndex& idx);

	/// @note Thread-safe against allocate(), deferredFree() and endFrame()
	void endFrame();

private:
	/// GPU scene allocator that emulates a contiguous array of elements. It's an array of objects of the same size.
	/// This array is a contiguous piece of memory. This helps same, in function, objects be close together.
	class ContiguousArrayAllocator
	{
		friend class AllGpuSceneContiguousArrays;

	public:
		~ContiguousArrayAllocator()
		{
			ANKI_ASSERT(m_nextSlotIndex == 0 && "Forgot to deallocate");
			for([[maybe_unused]] const SceneDynamicArray<U32>& arr : m_garbage)
			{
				ANKI_ASSERT(arr.getSize() == 0);
			}
		}

		void init(U32 initialArraySize, U16 objectSize, F32 arrayGrowRate)
		{
			ANKI_ASSERT(initialArraySize > 0);
			ANKI_ASSERT(objectSize > 0 && objectSize <= 256); // 256 is arbitary
			ANKI_ASSERT(arrayGrowRate > 1.0);
			m_objectSize = objectSize;
			m_growRate = arrayGrowRate;
			m_initialArraySize = initialArraySize;
		}

		void destroy();

		/// Allocate a new object and return its index in the array.
		/// @note It's thread-safe against itself, deferredFree and endFrame.
		U32 allocateObject();

		/// Safely free an index allocated by allocateObject.
		/// @note It's thread-safe against itself, allocateObject and endFrame.
		void deferredFree(U32 crntFrameIdx, U32 index);

		/// Call this every frame.
		/// @note It's thread-safe against itself, deferredFree and allocateObject.
		void collectGarbage(U32 newFrameIdx);

		PtrSize getArrayBase() const
		{
			return m_allocation.getOffset();
		}

		U32 getElementCount() const
		{
			return m_nextSlotIndex;
		}

	private:
		GpuSceneBufferAllocation m_allocation;

		SceneDynamicArray<U32> m_freeSlotStack;

		Array<SceneDynamicArray<U32>, kMaxFramesInFlight> m_garbage;

		mutable SpinLock m_mtx;

		F32 m_growRate = 2.0;
		U32 m_initialArraySize = 0;
		U16 m_objectSize = 0;

		U32 m_nextSlotIndex = 0;
	};

	Array<ContiguousArrayAllocator, U32(GpuSceneContiguousArrayType::kCount)> m_allocs;

	U8 m_frame = 0;

	static constexpr Array<U8, U32(GpuSceneContiguousArrayType::kCount)> m_componentCount = {
		2, kMaxLodCount, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
	static constexpr Array<U8, U32(GpuSceneContiguousArrayType::kCount)> m_componentSize = {
		sizeof(Mat3x4),
		sizeof(GpuSceneMeshLod),
		sizeof(GpuSceneParticleEmitter),
		sizeof(GpuScenePointLight),
		sizeof(GpuSceneSpotLight),
		sizeof(GpuSceneReflectionProbe),
		sizeof(GpuSceneGlobalIlluminationProbe),
		sizeof(GpuSceneDecal),
		sizeof(GpuSceneFogDensityVolume),
		sizeof(GpuSceneRenderable),
		sizeof(GpuSceneRenderableAabb),
		sizeof(GpuSceneRenderableAabb),
		sizeof(GpuSceneRenderableAabb)};

	AllGpuSceneContiguousArrays();

	~AllGpuSceneContiguousArrays();
};

inline GpuSceneContiguousArrayIndex::~GpuSceneContiguousArrayIndex()
{
	AllGpuSceneContiguousArrays::getSingleton().deferredFree(*this);
}

inline U32 GpuSceneContiguousArrayIndex::getOffsetInGpuScene() const
{
	return AllGpuSceneContiguousArrays::getSingleton().getElementOffsetInGpuScene(*this);
}
/// @}

} // end namespace anki
