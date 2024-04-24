// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Renderer/RendererObject.h>

namespace anki {

/// @addtogroup renderer
/// @{

/// Build acceleration structures.
class AccelerationStructureBuilder : public RendererObject
{
public:
	Error init()
	{
		return Error::kNone;
	}

	void populateRenderGraph(RenderingContext& ctx);

	AccelerationStructureHandle getAccelerationStructureHandle() const
	{
		return m_runCtx.m_tlasHandle;
	}

	void getVisibilityInfo(BufferHandle& handle, BufferView& buffer) const
	{
		handle = m_runCtx.m_visibilityHandle;
		buffer = m_runCtx.m_visibleRenderableIndicesBuff;
	}

public:
	class
	{
	public:
		AccelerationStructurePtr m_tlas;
		AccelerationStructureHandle m_tlasHandle;

		BufferHandle m_visibilityHandle;
		BufferView m_visibleRenderableIndicesBuff;
	} m_runCtx;
};
/// @}

} // namespace anki
