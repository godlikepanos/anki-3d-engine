#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/resource/Resource.h"
#include "anki/scene/SceneNode.h"

namespace anki {

class PassLevelKey;

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
public:
	/// The one and only constructor
	RenderableDrawer(Renderer* r_)
		: r(r_)
	{}

	void render(const Frustumable& fr,
		uint pass, Renderable& renderable);

private:
	Renderer* r;

	void setupShaderProg(
		const PassLevelKey& key,
		const Frustumable& fr,
		Renderable& renderable);
};

} // end namespace anki

#endif
