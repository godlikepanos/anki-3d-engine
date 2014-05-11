#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/util/StdTypes.h"
#include "anki/util/Ptr.h"
#include "anki/resource/RenderingKey.h"
#include "anki/scene/Forward.h"
#include "anki/Gl.h"

namespace anki {

// Forward
class Renderer;

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
	static const U32 MAX_UNIFORM_BUFFER_SIZE = 1024 * 1024 * 1;

	/// The one and only constructor
	RenderableDrawer(Renderer* r);

	void prepareDraw(RenderingStage stage, Pass pass, GlJobChainHandle& jobs);

	void render(
		SceneNode& frsn,
		VisibleNode& visible);

	void finishDraw();

private:
	Renderer* m_r;
	GlBufferHandle m_uniformBuff;

	/// @name State
	/// @{
	GlJobChainHandle m_jobs;
	U8* m_uniformPtr;

	/// Used to calc if the uni buffer is big enough. Zero it per swap buffers
	U32 m_uniformsUsedSize; 
	U32 m_uniformsUsedSizeFrame;

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
