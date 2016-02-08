// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/RenderingKey.h>
#include <anki/scene/Forward.h>
#include <anki/Gr.h>

namespace anki
{

// Forward
class Renderer;
class DrawContext;

/// @addtogroup renderer
/// @{

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
	friend class SetupRenderableVariableVisitor;
	friend class RenderTask;

public:
	RenderableDrawer(Renderer* r)
		: m_r(r)
	{
	}

	~RenderableDrawer();

	ANKI_USE_RESULT Error drawRange(Pass pass,
		const FrustumComponent& frc,
		CommandBufferPtr cmdb,
		VisibleNode* begin,
		VisibleNode* end);

private:
	Renderer* m_r;

	void setupUniforms(DrawContext& ctx,
		const RenderComponent& renderable,
		const RenderingKey& key);

	ANKI_USE_RESULT Error drawSingle(DrawContext& ctx);
};
/// @}

} // end namespace anki
