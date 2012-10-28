#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/util/StdTypes.h"

namespace anki {

class PassLevelKey;
class Renderer;
class Frustumable;
class Renderable;

/// It includes all the functions to render a Renderable
class RenderableDrawer
{
public:
	static const U UNIFORM_BLOCK_MAX_SIZE = 256;

	/// The one and only constructor
	RenderableDrawer(Renderer* r_)
		: r(r_)
	{}

	void prepareDraw()
	{}

	void render(const Frustumable& fr,
		U32 pass, Renderable& renderable);

private:
	Renderer* r;

	void setupShaderProg(
		const PassLevelKey& key,
		const Frustumable& fr,
		Renderable& renderable);
};

} // end namespace anki

#endif
