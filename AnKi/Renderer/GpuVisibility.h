// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Readback.h>
#include <AnKi/Resource/RenderingKey.h>

namespace anki {

/// @addtogroup renderer
/// @{

class GpuVisibilityOutput
{
public:
	BufferHandle m_mdiDrawCountsHandle; ///< Just expose one handle for depedencies. No need to track all other buffers.

	Buffer* m_instanceRateRenderablesBuffer;
	Buffer* m_drawIndexedIndirectArgsBuffer;
	Buffer* m_mdiDrawCountsBuffer;

	PtrSize m_instanceRateRenderablesBufferOffset;
	PtrSize m_drawIndexedIndirectArgsBufferOffset;
	PtrSize m_mdiDrawCountsBufferOffset;

	PtrSize m_instanceRateRenderablesBufferRange;
	PtrSize m_drawIndexedIndirectArgsBufferRange;
	PtrSize m_mdiDrawCountsBufferRange;
};

/// Performs GPU visibility for some pass.
class GpuVisibility : public RendererObject
{
public:
	Error init();

	/// Populate the rendergraph.
	void populateRenderGraph(CString passesName, RenderingTechnique technique, const Mat4& viewProjectionMat, Vec3 lodReferencePoint,
							 const Array<F32, kMaxLodCount - 1> lodDistances, const RenderTargetHandle* hzbRt, RenderGraphDescription& rgraph,
							 GpuVisibilityOutput& out);

private:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs;

#if ANKI_STATS_ENABLED
	Array<GpuReadbackMemoryAllocation, kMaxFramesInFlight> m_readbackMemory;
	U64 m_lastFrameIdx = kMaxU64;
#endif
};

/// @memberof GpuVisibilityNonRenderables
class GpuVisibilityNonRenderablesOutput
{
public:
	BufferHandle m_bufferHandle; ///< Some buffer handle to be used for tracking. No need to track all buffers.

	Array<Buffer*, U32(GpuSceneNonRenderableObjectType::kCount)> m_buffers = {};
	Array<PtrSize, U32(GpuSceneNonRenderableObjectType::kCount)> m_bufferOffsets = {};
	Array<PtrSize, U32(GpuSceneNonRenderableObjectType::kCount)> m_bufferRanges = {};
};

/// GPU visibility of lights, probes etc.
class GpuVisibilityNonRenderables : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(CString passesName, GpuSceneNonRenderableObjectTypeBit objectTypes, const Mat4& viewProjectionMat,
							 const RenderTargetHandle* hzbRt, RenderGraphDescription& rgraph, GpuVisibilityNonRenderablesOutput& out);

private:
	ShaderProgramResourcePtr m_prog;
	Array2d<ShaderProgramPtr, 2, U32(GpuSceneNonRenderableObjectType::kCount)> m_grProgs;
};
/// @}

} // end namespace anki
