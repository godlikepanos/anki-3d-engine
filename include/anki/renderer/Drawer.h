// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>
#include <anki/util/Ptr.h>
#include <anki/resource/RenderingKey.h>
#include <anki/scene/Forward.h>
#include <anki/Gr.h>

namespace anki
{

// Forward
class Renderer;
struct RenderContext;

/// @addtogroup renderer
/// @{

/// The rendering stage
enum class RenderingStage : U8
{
	MATERIAL,
	BLEND
};

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

	ANKI_USE_RESULT Error render(FrustumComponent& frc,
		RenderingStage stage,
		Pass pass,
		SArray<CommandBufferPtr>& cmdbs);

private:
	Renderer* m_r;

	void setupUniforms(RenderContext& ctx,
		const RenderComponent& renderable,
		const RenderingKey& key);

	ANKI_USE_RESULT Error renderSingle(RenderContext& ctx);
};
/// @}

} // end namespace anki
