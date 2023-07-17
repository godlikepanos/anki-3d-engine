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

/// @memberof GpuVisibility
class GpuVisibilityInput
{
public:
	CString m_passesName;
	RenderingTechnique m_technique = RenderingTechnique::kCount;
	Mat4 m_viewProjectionMatrix = Mat4::getIdentity();
	Vec3 m_lodReferencePoint = Vec3(0.0f);
	Array<F32, kMaxLodCount - 1> m_lodDistances = {};
	const RenderTargetHandle* m_hzbRt = nullptr; ///< Optional.
	RenderGraphDescription* m_rgraph = nullptr;
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

	/// Populate the rendergraph.
	void populateRenderGraph(GpuVisibilityInput& in, GpuVisibilityOutput& out);

private:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs;

#if ANKI_STATS_ENABLED
	Array<GpuReadbackMemoryAllocation, kMaxFramesInFlight> m_readbackMemory;
	U64 m_lastFrameIdx = kMaxU64;
#endif
};

/// @memberof GpuVisibilityNonRenderables
class GpuVisibilityNonRenderablesInput
{
public:
	CString m_passesName;
	GpuSceneNonRenderableObjectType m_objectType = GpuSceneNonRenderableObjectType::kCount;
	Mat4 m_viewProjectionMat;
	const RenderTargetHandle* m_hzbRt = nullptr;
	RenderGraphDescription* m_rgraph = nullptr;

	BufferOffsetRange m_cpuFeedbackBuffer; ///< Optional.
};

/// @memberof GpuVisibilityNonRenderables
class GpuVisibilityNonRenderablesOutput
{
public:
	BufferHandle m_bufferHandle; ///< Some buffer handle to be used for tracking. No need to track all buffers.
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

	static constexpr U32 kMaxPopulateRenderGraphPerFrame = 32; ///< Max times the populateRenderGraph() will be called per frame.

	Array<BufferPtr, kMaxPopulateRenderGraphPerFrame> m_counterBuffers; ///< A buffer containing multiple counters for atomic operations.
	U64 m_lastFrameIdx = kMaxU64;
	U32 m_runIdx = 0;
};
/// @}

} // end namespace anki
