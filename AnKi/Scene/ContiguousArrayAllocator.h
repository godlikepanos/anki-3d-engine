// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>

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

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GpuSceneContiguousArrayType)

/// Contains a number of contiguous array allocators for various GPU scene contiguous objects.
class AllGpuSceneContiguousArrays
{
public:
	using Index = U32;

	void init(SceneGraph* scene);

	void destroy();

	/// @note Thread-safe against allocate(), deferredFree() and endFrame()
	Index allocate(GpuSceneContiguousArrayType type);

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

	constexpr static U32 getElementSize(GpuSceneContiguousArrayType type)
	{
		return m_componentSize[type] * m_componentCount[type];
	}

	/// @note Thread-safe against allocate(), deferredFree() and endFrame()
	void deferredFree(GpuSceneContiguousArrayType type, Index idx);

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
			for([[maybe_unused]] const DynamicArray<Index>& arr : m_garbage)
			{
				ANKI_ASSERT(arr.getSize() == 0);
			}
			ANKI_ASSERT(m_poolToken.m_offset == kMaxPtrSize);
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

		void destroy(GpuSceneMemoryPool* gpuScene, HeapMemoryPool* cpuPool);

		/// Allocate a new object and return its index in the array.
		/// @note It's thread-safe against itself, deferredFree and endFrame.
		Index allocateObject(GpuSceneMemoryPool* gpuScene, HeapMemoryPool* cpuPool);

		/// Safely free an index allocated by allocateObject.
		/// @note It's thread-safe against itself, allocateObject and endFrame.
		void deferredFree(U32 crntFrameIdx, HeapMemoryPool* cpuPool, Index index);

		/// Call this every frame.
		/// @note It's thread-safe against itself, deferredFree and allocateObject.
		void collectGarbage(U32 newFrameIdx, GpuSceneMemoryPool* gpuScene, HeapMemoryPool* cpuPool);

		PtrSize getArrayBase() const
		{
			ANKI_ASSERT(m_poolToken.isValid());
			return m_poolToken.m_offset;
		}

		U32 getElementCount() const
		{
			return m_nextSlotIndex;
		}

	private:
		SegregatedListsGpuMemoryPoolToken m_poolToken;

		DynamicArray<Index> m_freeSlotStack;

		Array<DynamicArray<Index>, kMaxFramesInFlight> m_garbage;

		mutable SpinLock m_mtx;

		F32 m_growRate = 2.0;
		U32 m_initialArraySize = 0;
		U16 m_objectSize = 0;

		U32 m_nextSlotIndex = 0;
	};

	SceneGraph* m_scene = nullptr;

	Array<ContiguousArrayAllocator, U32(GpuSceneContiguousArrayType::kCount)> m_allocs;

	U8 m_frame = 0;

	static constexpr Array<U8, U32(GpuSceneContiguousArrayType::kCount)> m_componentCount = {
		2, kMaxLodCount, 1, 1, 1, 1, 1, 1, 1};
	static constexpr Array<U8, U32(GpuSceneContiguousArrayType::kCount)> m_componentSize = {
		sizeof(Mat3x4),
		sizeof(GpuSceneMeshLod),
		sizeof(GpuSceneParticleEmitter),
		sizeof(GpuScenePointLight),
		sizeof(GpuSceneSpotLight),
		sizeof(GpuSceneReflectionProbe),
		sizeof(GpuSceneGlobalIlluminationProbe),
		sizeof(GpuSceneDecal),
		sizeof(GpuSceneFogDensityVolume)};
};
/// @}

} // end namespace anki
