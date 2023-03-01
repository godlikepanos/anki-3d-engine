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

	kCount,
	kFirst = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(GpuSceneContiguousArrayType)

/// Contains a number of contiguous array allocators for various GPU scene contiguous objects.
class AllGpuSceneContiguousArrays
{
public:
	void init(SceneGraph* scene);

	void destroy();

	PtrSize allocate(GpuSceneContiguousArrayType type);

	void deferredFree(GpuSceneContiguousArrayType type, PtrSize offset);

	void endFrame();

private:
	/// GPU scene allocator that emulates a contiguous array of elements. It's an array of objects of the same size.
	/// This array is a contiguous piece of memory. This helps same, in function, objects be close together.
	class ContiguousArrayAllocator
	{
		friend class AllGpuSceneContiguousArrays;

	public:
		using Index = U32;

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

	private:
		SegregatedListsGpuMemoryPoolToken m_poolToken;

		DynamicArray<Index> m_freeSlotStack;

		Array<DynamicArray<Index>, kMaxFramesInFlight> m_garbage;

		SpinLock m_mtx;

		F32 m_growRate = 2.0;
		U32 m_initialArraySize = 0;
		U16 m_objectSize = 0;

		U32 m_nextSlotIndex = 0;
	};

	SceneGraph* m_scene = nullptr;

	Array<ContiguousArrayAllocator, U32(GpuSceneContiguousArrayType::kCount)> m_allocs;

	U8 m_frame = 0;

	static constexpr Array<U8, U32(GpuSceneContiguousArrayType::kCount)> m_componentCount = {2, kMaxLodCount, 1};
	static constexpr Array<U8, U32(GpuSceneContiguousArrayType::kCount)> m_componentSize = {
		sizeof(Mat3x4), sizeof(GpuSceneMeshLod), sizeof(GpuSceneParticleEmitter)};
};
/// @}

} // end namespace anki
