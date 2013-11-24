#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/util/StdTypes.h"
#include "anki/resource/PassLodKey.h"

namespace anki {

class Renderer;
class FrustumComponent;
class SceneNode;
class ShaderProgram;
class RenderComponent;
class Drawcall;
class VisibleNode;

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
public:
	enum RenderingStage
	{
		RS_MATERIAL,
		RS_BLEND
	};

	/// The one and only constructor
	RenderableDrawer(Renderer* r_)
		: r(r_)
	{}

	void prepareDraw()
	{}

	void render(
		SceneNode& frsn,
		RenderingStage stage, 
		Pass pass,
		VisibleNode& visible);

private:
	Renderer* r;

	void setupShaderProg(
		const PassLodKey& key,
		const FrustumComponent& fr,
		const ShaderProgram& prog,
		RenderComponent& renderable,
		VisibleNode& visible,
		F32 flod,
		Drawcall* dc);
};

} // end namespace anki

#endif
