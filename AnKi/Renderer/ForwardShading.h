// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>
#include <AnKi/Renderer/Utils/GpuVisibility.h>

namespace anki {

// Forward rendering stage. The objects that blend must be handled differently
class ForwardShading : public RendererObject
{
public:
	ForwardShading() = default;

	~ForwardShading() = default;

	Error init()
	{
		return Error::kNone;
	}

	void populateRenderGraph();

	void setDependencies(GraphicsRenderPass& pass);

	void run(RenderPassWorkContext& rgraphCtx);

	// Use some of the output for debug drawing.
	const GpuVisibilityOutput& getGpuVisibilityOutput() const
	{
		return m_runCtx.m_visOut;
	}

private:
	class
	{
	public:
		GpuVisibilityOutput m_visOut;
	} m_runCtx;
};

} // end namespace anki
