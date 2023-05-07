// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Hierarchical depth generator.
class Hzb : public RendererObject
{
public:
	Error init();

	void populateRenderGraph(RenderingContext& ctx);

	const RenderTargetHandle& getHzbRt() const
	{
		return m_runCtx.m_hzbRt;
	}

private:
	RenderTargetDescription m_hzbRtDescr;

	class
	{
	public:
		ShaderProgramResourcePtr m_prog;
		ShaderProgramPtr m_grProg;
	} m_clearHzb;

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
		RenderTargetHandle m_hzbRt;
	} m_runCtx;
};
/// @}

} // end namespace anki