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
///
/// The HZB is cleared with -0.0. Then the reprojection may replace the -0.0 of some texels with something in the range of [0.0, 1.0] where 0.0 is the
/// far and 1.0 the near. So after reprojection we have -0.0 for untouched, 1.0 for near and 0.0 for far. We do this because reprojection will use
/// atomic min on integers and -0.0 > 1.0 > 0.0 if seen as U32. Downscaling also uses min becase we want the farthest value. When testing against the
/// HZB we only need to bring depth back to normal which is 0.0 for near and 1.0 for far. So it's a plain 1.0-x. So far becomes 1.0, near is 0.0 and
/// untouched becomes 1.0 which transaltes to far and it's what we want.
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
	Array<RenderTargetDescription, kMaxShadowCascades> m_hzbShadowRtDescrs;

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
		Array<ShaderProgramPtr, kMaxShadowCascades + 1> m_grProgs;
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
		Array<RenderTargetHandle, kMaxShadowCascades> m_hzbShadowRts;
	} m_runCtx;
};
/// @}

} // end namespace anki
