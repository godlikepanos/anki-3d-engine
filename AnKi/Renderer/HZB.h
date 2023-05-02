// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Hierarchical Z buffer generator.
class HZB : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	const RenderTargetHandle& getHZBRt() const
	{
		return m_runCtx.m_HZBRt;
	}

private:
	RenderTargetDescription m_HZBRtDescr;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_clearHZB;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_reproj;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
		BufferPtr m_counterBuffer;
		Bool m_counterBufferZeroed = false;
	} m_mipmapping;

	class
	{
	public:
		RenderTargetHandle m_HZBRt;
	} m_runCtx;
};
/// @}

} // end namespace anki
