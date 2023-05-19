// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Resource/RenderingKey.h>

namespace anki {

/// @addtogroup renderer
/// @{

class GpuVisibilityOutput
{
public:
	BufferHandle m_instanceRateRenderablesHandle;
	BufferHandle m_drawIndexedIndirectArgsHandle;
	BufferHandle m_mdiDrawCountsHandle;

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
	void populateRenderGraph(RenderingTechnique technique, const Mat4& viewProjectionMat, Vec3 lodReferencePoint,
							 const Array<F32, kMaxLodCount - 1> lodDistances, const RenderTargetHandle* hzbRt, RenderGraphDescription& rgraph,
							 GpuVisibilityOutput& out) const;

private:
	ShaderProgramResourcePtr m_prog;
	Array<ShaderProgramPtr, 2> m_grProgs;
};
/// @}

} // end namespace anki
