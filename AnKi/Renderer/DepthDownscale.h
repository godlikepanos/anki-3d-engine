// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Gr.h>
#include <AnKi/Resource/ImageResource.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Downscales the depth buffer a few times.
class DepthDownscale : public RendererObject
{
public:
	DepthDownscale(Renderer* r)
		: RendererObject(r)
	{
	}

	~DepthDownscale();

	ANKI_USE_RESULT Error init();

	/// Import render targets
	void importRenderTargets(RenderingContext& ctx);

	/// Populate the rendergraph.
	void populateRenderGraph(RenderingContext& ctx);

	/// Return a FP color render target with hierarchical Z (min Z) in it's mips.
	RenderTargetHandle getHiZRt() const
	{
		return m_runCtx.m_hizRt;
	}

	U32 getMipmapCount() const
	{
		return m_mipCount;
	}

	void getClientDepthMapInfo(F32*& depthValues, U32& width, U32& height) const
	{
		width = m_lastMipSize.x();
		height = m_lastMipSize.y();
		ANKI_ASSERT(m_clientBuffer);
		m_clientBuffer->invalidate(0, MAX_PTR_SIZE);
		depthValues = static_cast<F32*>(m_clientBufferAddr);
	}

private:
	TexturePtr m_hizTex;
	Bool m_hizTexImportedOnce = false;

	ShaderProgramResourcePtr m_prog;
	ShaderProgramPtr m_grProg;

	BufferPtr m_counterBuffer;
	Bool m_counterBufferZeroed = false;

	SamplerPtr m_reductionSampler;

	BufferPtr m_clientBuffer;
	void* m_clientBufferAddr = nullptr;

	UVec2 m_lastMipSize;
	U32 m_mipCount = 0;

	class
	{
	public:
		RenderTargetHandle m_hizRt;
	} m_runCtx; ///< Run context.

	ANKI_USE_RESULT Error initInternal();

	Bool doesSamplerReduction() const
	{
		return m_reductionSampler.isCreated();
	}
};
/// @}

} // end namespace anki
