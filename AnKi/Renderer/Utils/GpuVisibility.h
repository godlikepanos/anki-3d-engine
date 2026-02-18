// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/Readback.h>
#include <AnKi/Resource/RenderingKey.h>
#include <AnKi/GpuMemory/TextureMemoryPool.h>

namespace anki {

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

class BaseGpuVisibilityInput
{
public:
	CString m_passesName;
	RenderingTechnique m_technique = RenderingTechnique::kCount;

	Vec3 m_lodReferencePoint = Vec3(kMaxF32);
	Array<F32, kMaxLodCount - 1> m_lodDistances = {};

	RenderGraphBuilder* m_rgraph = nullptr;

	Bool m_gatherAabbIndices = false; // For debug draw.
	Bool m_hashVisibles = false; // Create a hash for the visible renderables.

	Bool m_limitMemory = false; // Use less memory but you pay some cost scheduling the work.
};

class FrustumGpuVisibilityInput : public BaseGpuVisibilityInput
{
public:
	Mat4 m_viewProjectionMatrix;

	/// The size of the viewport the visibility results will be used on. Used to kill objects that don't touch the sampling positions.
	UVec2 m_viewportSize;

	const RenderTargetHandle* m_hzbRt = nullptr; // Optional.

	Bool m_twoPhaseOcclusionCulling = false; // If it's false then it's only a single phase. Only applies when meshlet rendering is enabled.
};

class DistanceGpuVisibilityInput : public BaseGpuVisibilityInput
{
public:
	Vec3 m_pointOfTest = Vec3(0.0f);
	F32 m_testRadius = 1.0f;
};

class GpuVisibilityOutput
{
	friend class GpuVisibility;

public:
	BufferHandle m_dependency; // Just expose one handle for depedencies. No need to track all buffers. Wait on it using indirect draw usage.

	class
	{
	public:
		BufferView m_perDrawDataBuffer; // An array of GpuScenePerDraw.
		BufferView m_mdiDrawCountsBuffer; // An array of U32, one for each render state bucket (even those that use task/mesh flow).
		BufferView m_drawIndexedIndirectArgsBuffer; // Array of DrawIndexedIndirectArgs or DrawIndirectArgs.

		// One for each bucket. It's an index to the m_perDrawDataBuffer. It basically gives the offset to the m_perDrawDataBuffer for each bucket
		BufferView m_firstPerDrawBuffer;

		// Defines the element sub-ranges in the m_drawIndexedIndirectArgsBuffer per render state bucket.
		WeakArray<InstanceRange> m_bucketIndirectArgsRanges;
	} m_legacy; // Legacy vertex shading.

	class
	{
	public:
		BufferView m_dispatchMeshIndirectArgsBuffer; // H/W meshlet rendering array of DispatchIndirectArgs, one for each render state bucket.
		BufferView m_drawIndirectArgs; // S/W meshlet rendering array of DrawIndirectArgs, one for each state bucket.

		BufferView m_meshletInstancesBuffer;

		BufferView m_firstMeshletBuffer; // For H/W meshlet rendering. Points to the first meshlet in the m_meshletInstancesBuffer. One per bucket.
	} m_mesh; // S/W or H/W meshlet rendering.

	// [Optional] Indices to the AABB buffer (LodAndGpuSceneRenderableBoundingVolumeIndex). The 1st element is the count
	BufferView m_visibleAaabbIndicesBuffer;

	BufferView m_visiblesHashBuffer; // [Optional] A hash of the visible objects. Used to conditionaly not perform shadow randering.

	Bool containsDrawcalls() const
	{
		return m_dependency.isValid();
	}

private:
	class
	{
	public:
		BufferView m_meshletsFailedHzb;
		BufferView m_counters;
		BufferView m_meshletPrefixSums;
		BufferView m_gpuVisIndirectDispatchArgs;
	} m_stage1And2Mem; // Output of the 2nd (or 1st) stage that will be used in the 3rd

	class
	{
	public:
		BufferView m_indirectDrawArgs;
		BufferView m_dispatchMeshIndirectArgs;
		BufferView m_meshletInstances;
	} m_stage3Mem; // Output of the 3rd stage.
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

	/// Perform the optional stage 3: 2nd phase of the 2-phase occlusion culling.
	/// @note Not thread-safe.
	void populateRenderGraphStage3(FrustumGpuVisibilityInput& in, GpuVisibilityOutput& out);

	/// Perform simple distance-based visibility testing.
	/// @note Not thread-safe.
	void populateRenderGraph(DistanceGpuVisibilityInput& in, GpuVisibilityOutput& out)
	{
		populateRenderGraphInternal(true, in, out);
	}

private:
	ShaderProgramResourcePtr m_1stStageProg;
	Array5d<ShaderProgramPtr, 2, 2, 2, 2, 2> m_frustumGrProgs;
	Array4d<ShaderProgramPtr, 2, 2, 2, 2> m_distGrProgs;

	ShaderProgramResourcePtr m_2ndStageProg;
	ShaderProgramPtr m_gatherGrProg;
	Array4d<ShaderProgramPtr, 2, 2, 2, 2> m_meshletGrProgs;

	class
	{
	public:
		class
		{
		public:
			BufferView m_visibleRenderables;
			BufferView m_visibleMeshlets;
		} m_stage1;

		class
		{
		public:
			BufferView m_perDraw;
			BufferView m_drawIndexedIndirectArgs;
		} m_stage2Legacy;

		class
		{
		public:
			BufferView m_meshletInstances;
			BufferView m_meshletsFailedHzb;
		} m_stage2Meshlet;

		class
		{
		public:
			BufferView m_meshletInstances;
		} m_stage3;

		U64 m_frameIdx = kMaxU64;

		BufferHandle m_dep;
	} m_persistentMemory;

	MultiframeReadbackToken m_outOfMemoryReadback;
	BufferView m_outOfMemoryReadbackBuffer;

	void populateRenderGraphInternal(Bool distanceBased, BaseGpuVisibilityInput& in, GpuVisibilityOutput& out);
};

class GpuVisibilityNonRenderablesInput
{
public:
	CString m_passesName;
	GpuSceneNonRenderableObjectType m_objectType = GpuSceneNonRenderableObjectType::kCount;
	Mat4 m_viewProjectionMat;
	RenderGraphBuilder* m_rgraph = nullptr;

	const RenderTargetHandle* m_hzbRt = nullptr; // Optional.
	BufferView m_cpuFeedbackBuffer; // Optional.
};

class GpuVisibilityNonRenderablesOutput
{
public:
	BufferHandle m_visiblesBufferHandle; // Buffer handle holding the visible objects. Used for tracking. No need to track all buffers.
	BufferView m_visiblesBuffer;
};

// GPU visibility of lights, probes etc.
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
	TextureMemoryPoolAllocation m_counterBuffer; // A buffer containing multiple counters for atomic operations.
	U64 m_lastFrameIdx = kMaxU64;
	U32 m_counterBufferOffset = 0;
};

class GpuVisibilityAccelerationStructuresInput
{
public:
	CString m_passesName;

	Vec3 m_lodReferencePoint = Vec3(kMaxF32);
	Array<F32, kMaxLodCount - 1> m_lodDistances = {};

	Vec3 m_pointOfTest = Vec3(kMaxF32);
	F32 m_testRadius = kMaxF32;

	Mat4 m_viewProjectionMatrix;

	RenderGraphBuilder* m_rgraph = nullptr;

	void validate() const
	{
		ANKI_ASSERT(m_passesName.getLength() > 0);
		ANKI_ASSERT(m_lodReferencePoint.x != kMaxF32);
		ANKI_ASSERT(m_lodReferencePoint == m_pointOfTest && "For now these should be the same");
		ANKI_ASSERT(m_testRadius != kMaxF32);
		ANKI_ASSERT(m_viewProjectionMatrix != Mat4());
		ANKI_ASSERT(m_rgraph);
	}
};

class GpuVisibilityAccelerationStructuresOutput
{
public:
	BufferHandle m_dependency; // Some handle to track dependencies. No need to track every buffer.

	BufferView m_instancesBuffer; // Points to AccelerationStructureBuildRangeInfo::m_primitiveCount number of AccelerationStructureInstance.
	BufferView m_renderablesBuffer; // AccelerationStructureBuildRangeInfo::m_primitiveCount + 1 number of indices to renderables.

	BufferView m_buildSbtIndirectArgsBuffer; // The DispatchIndirectArgs for the SBT dispatches.
};

// Performs visibility to gather bottom-level acceleration structures in a buffer that can be used to build a TLAS.
class GpuVisibilityAccelerationStructures : public RendererObject
{
public:
	Error init();

	void pupulateRenderGraph(GpuVisibilityAccelerationStructuresInput& in, GpuVisibilityAccelerationStructuresOutput& out);

private:
	ShaderProgramResourcePtr m_visibilityProg;
	ShaderProgramPtr m_visibilityGrProg;
	ShaderProgramPtr m_zeroRemainingInstancesGrProg;

	TextureMemoryPoolAllocation m_counterBuffer; // A buffer containing multiple counters for atomic operations.

#if ANKI_ASSERTIONS_ENABLED
	U64 m_lastFrameIdx = kMaxU64;
#endif
};

class GpuVisibilityLocalLightsInput
{
public:
	UVec3 m_cellCounts;
	Vec3 m_cellSize;

	Vec3 m_cameraPosition;
	Vec3 m_lookDirection;

	U32 m_lightIndexListSize = 0; // The number of light indices to store.

	CString m_passesName = "GpuVisibilityLocalLights";

	RenderGraphBuilder* m_rgraph = nullptr;
};

class GpuVisibilityLocalLightsOutput
{
public:
	BufferHandle m_dependency; // Some handle to track dependencies. No need to track every buffer.

	BufferView m_lightIndexOffsetsPerCellBuffer; // One offset to the m_lightIndexBuffer. One offset per cell.
	BufferView m_lightIndexCountsPerCellBuffer; // Number of lights per cell.
	BufferView m_lightIndexListBuffer; // Contains indexes to the GPU scene lights array.

	// The volume of the grid.
	Vec3 m_lightGridMin;
	Vec3 m_lightGridMax;
};

// Gathers the local lights around the camera to a grid.
class GpuVisibilityLocalLights : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(GpuVisibilityLocalLightsInput& in, GpuVisibilityLocalLightsOutput& out);

private:
	static constexpr F32 kForwardBias = 4.0f;

	ShaderProgramResourcePtr m_visibilityProg;

	ShaderProgramPtr m_setupGrProg;
	ShaderProgramPtr m_countGrProg;
	ShaderProgramPtr m_prefixSumGrProg;
	ShaderProgramPtr m_fillGrProg;
};

} // end namespace anki
