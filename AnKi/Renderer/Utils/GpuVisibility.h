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

	Vec3 m_lodReferencePoint = Vec3(0.0f);
	Array<F32, kMaxLodCount - 1> m_lodDistances = {};

	RenderGraphDescription* m_rgraph = nullptr;
};

/// @memberof GpuVisibility
class FrustumGpuVisibilityInput : public BaseGpuVisibilityInput
{
public:
	Mat4 m_viewProjectionMatrix = Mat4::getIdentity();
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
	BufferHandle m_mdiDrawCountsHandle; ///< Just expose one handle for depedencies. No need to track all other buffers.

	BufferOffsetRange m_instanceRateRenderablesBuffer;
	BufferOffsetRange m_drawIndexedIndirectArgsBuffer;
	BufferOffsetRange m_mdiDrawCountsBuffer;
};

/// Performs GPU visibility for some pass.
class GpuVisibility : public RendererObject
{
public:
	Error init();

	/// Perform frustum visibility testing.
	void populateRenderGraph(FrustumGpuVisibilityInput& in, GpuVisibilityOutput& out)
	{
		populateRenderGraphInternal(false, in, out);
	}

	/// Perform simple distance-based visibility testing.
	void populateRenderGraph(DistanceGpuVisibilityInput& in, GpuVisibilityOutput& out)
	{
		populateRenderGraphInternal(true, in, out);
	}

private:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_frustumGrProgs;
	ShaderProgramPtr m_distGrProg;

#if ANKI_STATS_ENABLED
	Array<GpuReadbackMemoryAllocation, kMaxFramesInFlight> m_readbackMemory;
	U64 m_lastFrameIdx = kMaxU64;
#endif

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
/// @}

} // end namespace anki
