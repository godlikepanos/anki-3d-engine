// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
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
	AccelerationStructureBuilder(Renderer* r)
		: RendererObject(r)
	{
	}

	~AccelerationStructureBuilder()
	{
	}

	Error init()
	{
		return Error::kNone;
	}

	void populateRenderGraph(RenderingContext& ctx);

	AccelerationStructureHandle getAccelerationStructureHandle() const
	{
		return m_runCtx.m_tlasHandle;
	}

public:
	class
	{
	public:
		AccelerationStructurePtr m_tlas;
		AccelerationStructureHandle m_tlasHandle;
	} m_runCtx;
};
/// @}

} // namespace anki
