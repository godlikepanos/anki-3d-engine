// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/util/StdTypes.h"
#include "anki/util/Ptr.h"
#include "anki/resource/RenderingKey.h"
#include "anki/scene/Forward.h"
#include "anki/Gr.h"

namespace anki {

// Forward
class Renderer;
class SetupRenderableVariableVisitor;

/// @addtogroup renderer
/// @{

/// The rendering stage
enum class RenderingStage: U8
{
	MATERIAL,
	BLEND
};

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
	friend class SetupRenderableVariableVisitor;

public:
	RenderableDrawer();

	~RenderableDrawer();

	/// The one and only constructor
	ANKI_USE_RESULT Error create(Renderer* r);

	void prepareDraw(
		RenderingStage stage, Pass pass, CommandBufferPtr& cmdBuff);

	ANKI_USE_RESULT Error render(
		SceneNode& frsn,
		VisibleNode& visible);

	void finishDraw();

private:
	Renderer* m_r;
	UniquePtr<SetupRenderableVariableVisitor> m_variableVisitor;

	/// @name State
	/// @{
	CommandBufferPtr m_cmdBuff;

	RenderingStage m_stage;
	Pass m_pass;
	/// @}

	void setupUniforms(
		VisibleNode& visibleNode,
		RenderComponent& renderable,
		FrustumComponent& fr,
		F32 flod);
};

/// @}

} // end namespace anki

#endif
