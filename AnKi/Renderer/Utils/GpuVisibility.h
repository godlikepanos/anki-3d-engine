// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/Readback.h>
#include <AnKi/Resource/RenderingKey.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// @memberof GpuVisibility
class InstanceRange
{
	friend class GpuVisibility;

public:
	U32 getFirstInstance() const
	{
		ANKI_ASSERT(isValid());
		return m_firstInstance;
	}

	U32 getInstanceCount() const
	{
		ANKI_ASSERT(isValid());
		return m_instanceCount;
	}

	Bool isValid() const
	{
		return m_instanceCount > 0;
	}

private:
	U32 m_firstInstance = 0;
	U32 m_instanceCount = 0;
};

/// @memberof GpuVisibility
class BaseGpuVisibilityInput
{
public:
	CString m_passesName;
	RenderingTechnique m_technique = RenderingTechnique::kCount;

	Vec3 m_lodReferencePoint = Vec3(kMaxF32);
	Array<F32, kMaxLodCount - 1> m_lodDistances = {};

	RenderGraphDescription* m_rgraph = nullptr;

	Bool m_gatherAabbIndices = false; ///< For debug draw.
	Bool m_hashVisibles = false; ///< Create a hash for the visible renderables.
};

/// @memberof GpuVisibility
class FrustumGpuVisibilityInput : public BaseGpuVisibilityInput
{
public:
	Mat4 m_viewProjectionMatrix;

	/// The size of the viewport the visibility results will be used on. Used to kill objects that don't touch the sampling positions.
	UVec2 m_viewportSize;

	const RenderTargetHandle* m_hzbRt = nullptr; ///< Optional.
};

/// @memberof GpuVisibility
class DistanceGpuVisibilityInput : public BaseGpuVisibilityInput
{
public:
	Vec3 m_pointOfTest = Vec3(0.0f);
	F32 m_testRadius = 1.0f;
};

/// @memberof GpuVisibility
class GpuVisibilityOutput
{
public:
	BufferHandle m_dependency; ///< Just expose one handle for depedencies. No need to track all buffers. Wait on it using indirect draw usage.

	class
	{
	public:
		BufferView m_renderableInstancesBuffer; ///< An array of GpuSceneRenderableInstance.
		BufferView m_mdiDrawCountsBuffer; ///< An array of U32, one for each render state bucket (even those that use task/mesh flow).
		BufferView m_drawIndexedIndirectArgsBuffer; ///< Array of DrawIndexedIndirectArgs or DrawIndirectArgs.

		/// Defines the element sub-ranges in the m_renderableInstancesBuffer an m_drawIndexedIndirectArgsBuffer per render state bucket.
		ConstWeakArray<InstanceRange> m_bucketRenderableInstanceRanges;
	} m_legacy; ///< Legacy vertex shading.

	class
	{
	public:
		BufferView m_taskShaderIndirectArgsBuffer; ///< An array of DispatchIndirectArgs, one for each render state bucket.
		BufferView m_meshletGroupInstancesBuffer; ///< Array with GpuSceneMeshletGroupInstance.

		/// Defines the element sub-ranges in the m_meshletGroupInstancesBuffer per render state bucket.
		ConstWeakArray<InstanceRange> m_bucketMeshletGroupInstanceRanges;
	} m_mesh; ///< S/W meshlets or H/W mesh shading.

	BufferView m_visibleAaabbIndicesBuffer; ///< [Optional] Indices to the AABB buffer. The 1st element is the count.

	BufferView m_visiblesHashBuffer; ///< [Optional] A hash of the visible objects. Used to conditionaly not perform shadow randering.

	Bool containsDrawcalls() const
	{
		return m_dependency.isValid();
	}
};

/// @memberof GpuVisibility
class BaseGpuMeshletVisibilityInput
{
public:
	CString m_passesName;

	RenderingTechnique m_technique = RenderingTechnique::kCount;

	BufferView m_taskShaderIndirectArgsBuffer; ///< Taken from GpuVisibilityOutput.
	BufferView m_meshletGroupInstancesBuffer; ///< Taken from GpuVisibilityOutput.
	ConstWeakArray<InstanceRange> m_bucketMeshletGroupInstanceRanges; ///< Taken from GpuVisibilityOutput.

	BufferHandle m_dependency;

	RenderGraphDescription* m_rgraph = nullptr;

	void fillBuffers(const GpuVisibilityOutput& perObjVisOut)
	{
		m_taskShaderIndirectArgsBuffer = perObjVisOut.m_mesh.m_taskShaderIndirectArgsBuffer;
		m_meshletGroupInstancesBuffer = perObjVisOut.m_mesh.m_meshletGroupInstancesBuffer;
		m_bucketMeshletGroupInstanceRanges = perObjVisOut.m_mesh.m_bucketMeshletGroupInstanceRanges;
		m_dependency = perObjVisOut.m_dependency;
	}
};

/// @memberof GpuVisibility
class GpuMeshletVisibilityInput : public BaseGpuMeshletVisibilityInput
{
public:
	Mat4 m_viewProjectionMatrix;
	Mat3x4 m_cameraTransform;

	/// The size of the viewport the visibility results will be used on. Used to kill objects that don't touch the sampling positions.
	UVec2 m_viewportSize;

	RenderTargetHandle m_hzbRt; ///< Optional.
};

/// @memberof GpuVisibility
class PassthroughGpuMeshletVisibilityInput : public BaseGpuMeshletVisibilityInput
{
};

/// @memberof GpuVisibility
class GpuMeshletVisibilityOutput
{
public:
	BufferView m_drawIndirectArgsBuffer; ///< Array of DrawIndirectArgs. One for every render state bucket (even those that use that flow).
	BufferView m_meshletInstancesBuffer; ///< Array of GpuSceneMeshletInstance.

	/// Defines the element sub-ranges in the m_meshletInstancesBuffer per render state bucket.
	ConstWeakArray<InstanceRange> m_bucketMeshletInstanceRanges;

	BufferHandle m_dependency; ///< Some dependency to wait on. Wait usage is indirect draw.

	Bool isFilled() const
	{
		return m_dependency.isValid();
	}
};

/// Performs GPU visibility for some pass.
class GpuVisibility : public RendererObject
{
public:
	Error init();

	/// Perform frustum visibility testing.
	/// @note Not thread-safe.
	void populateRenderGraph(FrustumGpuVisibilityInput& in, GpuVisibilityOutput& out)
	{
		ANKI_ASSERT(in.m_viewProjectionMatrix != Mat4::getZero());
		ANKI_ASSERT(in.m_viewportSize != UVec2(0u));
		populateRenderGraphInternal(false, in, out);
	}

	/// Perform simple distance-based visibility testing.
	/// @note Not thread-safe.
	void populateRenderGraph(DistanceGpuVisibilityInput& in, GpuVisibilityOutput& out)
	{
		populateRenderGraphInternal(true, in, out);
	}

	/// Perform meshlet GPU visibility.
	/// @note Not thread-safe.
	void populateRenderGraph(GpuMeshletVisibilityInput& in, GpuMeshletVisibilityOutput& out)
	{
		populateRenderGraphMeshletInternal(false, in, out);
	}

	/// Perform meshlet GPU visibility.
	/// @note Not thread-safe.
	void populateRenderGraph(PassthroughGpuMeshletVisibilityInput& in, GpuMeshletVisibilityOutput& out)
	{
		populateRenderGraphMeshletInternal(true, in, out);
	}

private:
	ShaderProgramResourcePtr m_prog;
	Array4d<ShaderProgramPtr, 2, 2, 2, 3> m_frustumGrProgs;
	Array3d<ShaderProgramPtr, 2, 2, 3> m_distGrProgs;

	ShaderProgramResourcePtr m_meshletCullingProg;
	Array2d<ShaderProgramPtr, 2, 2> m_meshletCullingGrProgs;

	// Contains quite large buffer that we want want to reuse muptiple times in a single frame.
	class PersistentMemory
	{
	public:
		// Legacy
		BufferView m_drawIndexedIndirectArgsBuffer;
		BufferView m_renderableInstancesBuffer; ///< Instance rate vertex buffer.

		// HW & SW Meshlet rendering
		BufferView m_meshletGroupsInstancesBuffer;

		// SW meshlet rendering
		BufferView m_meshletInstancesBuffer; ///< Instance rate vertex buffer.

		BufferHandle m_bufferDepedency;
	};

	class PersistentMemoryMeshletRendering
	{
	public:
		// SW meshlet rendering
		BufferView m_meshletInstancesBuffer; ///< Instance rate vertex buffer.

		BufferHandle m_bufferDepedency;
	};

	class MemoryRequirements
	{
	public:
		U32 m_renderableInstanceCount = 0; ///< Count of GpuSceneRenderableInstance and a few other things
		U32 m_meshletGroupInstanceCount = 0; ///< Count of GpuSceneMeshletGroupInstance
		U32 m_meshletInstanceCount = 0; ///< Count of GpuSceneMeshletInstance

		MemoryRequirements max(const MemoryRequirements& b)
		{
			MemoryRequirements out;
#define ANKI_MAX(member) out.member = anki::max(member, b.member)
			ANKI_MAX(m_renderableInstanceCount);
			ANKI_MAX(m_meshletGroupInstanceCount);
			ANKI_MAX(m_meshletInstanceCount);
#undef ANKI_MAX
			return out;
		}
	};

	class
	{
	public:
		U64 m_frameIdx = kMaxU64;
		U32 m_populateRenderGraphCallCount = 0;
		U32 m_populateRenderGraphMeshletRenderingCallCount = 0;

		/// The more persistent memory there is the more passes will be able to run in parallel but the more memory is used.
		Array<PersistentMemory, 4> m_persistentMem;
		Array<PersistentMemoryMeshletRendering, 4> m_persistentMeshletRenderingMem; ///< See m_persistentMem.

		Array<MemoryRequirements, U32(RenderingTechnique::kCount)> m_totalMemRequirements;

		Array<WeakArray<InstanceRange>, U32(RenderingTechnique::kCount)> m_renderableInstanceRanges;
		Array<WeakArray<InstanceRange>, U32(RenderingTechnique::kCount)> m_meshletGroupInstanceRanges;
		Array<WeakArray<InstanceRange>, U32(RenderingTechnique::kCount)> m_meshletInstanceRanges;
	} m_runCtx;

	void populateRenderGraphInternal(Bool distanceBased, BaseGpuVisibilityInput& in, GpuVisibilityOutput& out);

	void populateRenderGraphMeshletInternal(Bool passthrough, BaseGpuMeshletVisibilityInput& in, GpuMeshletVisibilityOutput& out);

	static void computeGpuVisibilityMemoryRequirements(RenderingTechnique t, MemoryRequirements& total, WeakArray<MemoryRequirements> perBucket);
};

/// @memberof GpuVisibilityNonRenderables
class GpuVisibilityNonRenderablesInput
{
public:
	CString m_passesName;
	GpuSceneNonRenderableObjectType m_objectType = GpuSceneNonRenderableObjectType::kCount;
	Mat4 m_viewProjectionMat;
	RenderGraphDescription* m_rgraph = nullptr;

	const RenderTargetHandle* m_hzbRt = nullptr; ///< Optional.
	BufferView m_cpuFeedbackBuffer; ///< Optional.
};

/// @memberof GpuVisibilityNonRenderables
class GpuVisibilityNonRenderablesOutput
{
public:
	BufferHandle m_visiblesBufferHandle; ///< Buffer handle holding the visible objects. Used for tracking. No need to track all buffers.
	BufferView m_visiblesBuffer;
};

/// GPU visibility of lights, probes etc.
class GpuVisibilityNonRenderables : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(GpuVisibilityNonRenderablesInput& in, GpuVisibilityNonRenderablesOutput& out);

private:
	ShaderProgramResourcePtr m_prog;
	Array3d<ShaderProgramPtr, 2, U32(GpuSceneNonRenderableObjectType::kCount), 2> m_grProgs;

	static constexpr U32 kInitialCounterArraySize = 32;

	BufferHandle m_counterBufferZeroingHandle;
	BufferPtr m_counterBuffer; ///< A buffer containing multiple counters for atomic operations.
	U64 m_lastFrameIdx = kMaxU64;
	U32 m_counterBufferOffset = 0;
};

/// @memberof GpuVisibilityAccelerationStructures
class GpuVisibilityAccelerationStructuresInput
{
public:
	CString m_passesName;

	Vec3 m_lodReferencePoint = Vec3(kMaxF32);
	Array<F32, kMaxLodCount - 1> m_lodDistances = {};

	Vec3 m_pointOfTest = Vec3(kMaxF32);
	F32 m_testRadius = kMaxF32;

	Mat4 m_viewProjectionMatrix;

	RenderGraphDescription* m_rgraph = nullptr;

	void validate() const
	{
		ANKI_ASSERT(m_passesName.getLength() > 0);
		ANKI_ASSERT(m_lodReferencePoint.x() != kMaxF32);
		ANKI_ASSERT(m_lodReferencePoint == m_pointOfTest && "For now these should be the same");
		ANKI_ASSERT(m_testRadius != kMaxF32);
		ANKI_ASSERT(m_viewProjectionMatrix != Mat4());
		ANKI_ASSERT(m_rgraph);
	}
};

/// @memberof GpuVisibilityAccelerationStructures
class GpuVisibilityAccelerationStructuresOutput
{
public:
	BufferHandle m_someBufferHandle; ///< Some handle to track dependencies. No need to track every buffer.

	BufferView m_instancesBuffer; ///< Points to AccelerationStructureBuildRangeInfo::m_primitiveCount number of AccelerationStructureInstance.
	BufferView m_renderableIndicesBuffer; ///< AccelerationStructureBuildRangeInfo::m_primitiveCount number of indices to renderables.
};

/// Performs visibility to gather bottom-level acceleration structures in a buffer that can be used to build a TLAS.
class GpuVisibilityAccelerationStructures : public RendererObject
{
public:
	Error init();

	void pupulateRenderGraph(GpuVisibilityAccelerationStructuresInput& in, GpuVisibilityAccelerationStructuresOutput& out);

private:
	ShaderProgramResourcePtr m_visibilityProg;
	ShaderProgramPtr m_visibilityGrProg;

	ShaderProgramResourcePtr m_zeroRemainingInstancesProg;
	ShaderProgramPtr m_zeroRemainingInstancesGrProg;

	BufferPtr m_counterBuffer; ///< A buffer containing multiple counters for atomic operations.

#if ANKI_ASSERTIONS_ENABLED
	U64 m_lastFrameIdx = kMaxU64;
#endif
};
/// @}

} // end namespace anki
