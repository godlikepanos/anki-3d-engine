// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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

	/// The size of the render target the visibility results will be used on. Used to kill objects that don't touch the sampling positions.
	UVec2 m_finalRenderTargetSize;

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
	BufferHandle m_someBufferHandle; ///< Just expose one handle for depedencies. No need to track all buffers.

	BufferOffsetRange m_instanceRateRenderablesBuffer; ///< An array of GpuSceneRenderableVertex.
	BufferOffsetRange m_drawIndexedIndirectArgsBuffer; ///< An array of DrawIndexedIndirectArgs.
	BufferOffsetRange m_mdiDrawCountsBuffer; ///< An array of U32, one for each render state bucket (even those that use task/mesh flow).

	/// An array of DispatchIndirectArgs, one for each render state bucket (even those that use vertex flow).
	BufferOffsetRange m_taskShaderIndirectArgsBuffer;
	BufferOffsetRange m_taskShaderPayloadBuffer; ///< The payloads of task shaders. One for each task shader threadgroup.

	BufferOffsetRange m_visibleAaabbIndicesBuffer; ///< [Optional] Indices to the AABB buffer. The 1st element is the count.

	BufferOffsetRange m_visiblesHashBuffer; ///< [Optional] A hash of the visible objects. Used to conditionaly not perform shadow randering.
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
		ANKI_ASSERT(in.m_finalRenderTargetSize != UVec2(0u));
		populateRenderGraphInternal(false, in, out);
	}

	/// Perform simple distance-based visibility testing.
	/// @note Not thread-safe.
	void populateRenderGraph(DistanceGpuVisibilityInput& in, GpuVisibilityOutput& out)
	{
		populateRenderGraphInternal(true, in, out);
	}

private:
	ShaderProgramResourcePtr m_prog;
	Array3d<ShaderProgramPtr, 2, 2, 2> m_frustumGrProgs;
	Array2d<ShaderProgramPtr, 2, 2> m_distGrProgs;

	class
	{
	public:
		U64 m_frameIdx = kMaxU64;

		// Buffers bellow are quite large that's why want to reuse them muptiple times in a single frame.
		BufferOffsetRange m_drawIndexedIndirectArgsBuffer;
		BufferOffsetRange m_instanceRateRenderablesBuffer;
		BufferOffsetRange m_taskShaderPayloadBuffer;

		BufferHandle m_bufferDepedency;
	} m_runCtx;

	class Counts
	{
	public:
		U32 m_aabbCount;
		U32 m_bucketCount;
		U32 m_legacyGeometryFlowUserCount;
		U32 m_modernGeometryFlowUserCount;
		U32 m_meshletGroupCount;
		U32 m_allUserCount;

		Counts max(const Counts& b) const
		{
			Counts out;
#define ANKI_MAX(member) out.member = anki::max(member, b.member)
			ANKI_MAX(m_aabbCount);
			ANKI_MAX(m_bucketCount);
			ANKI_MAX(m_legacyGeometryFlowUserCount);
			ANKI_MAX(m_modernGeometryFlowUserCount);
			ANKI_MAX(m_meshletGroupCount);
			ANKI_MAX(m_allUserCount);
#undef ANKI_MAX
			return out;
		}
	};

	Counts countTechnique(RenderingTechnique t);

	void populateRenderGraphInternal(Bool distanceBased, BaseGpuVisibilityInput& in, GpuVisibilityOutput& out);
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
	BufferOffsetRange m_cpuFeedbackBuffer; ///< Optional.
};

/// @memberof GpuVisibilityNonRenderables
class GpuVisibilityNonRenderablesOutput
{
public:
	BufferHandle m_visiblesBufferHandle; ///< Buffer handle holding the visible objects. Used for tracking. No need to track all buffers.
	BufferOffsetRange m_visiblesBuffer;
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

	BufferOffsetRange m_instancesBuffer; ///< Points to AccelerationStructureBuildRangeInfo::m_primitiveCount number of AccelerationStructureInstance.
	BufferOffsetRange m_renderableIndicesBuffer; ///< AccelerationStructureBuildRangeInfo::m_primitiveCount number of indices to renderables.
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
