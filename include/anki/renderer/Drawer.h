#ifndef ANKI_RENDERER_DRAWER_H
#define ANKI_RENDERER_DRAWER_H

#include "anki/util/StdTypes.h"
#include "anki/resource/PassLevelKey.h"

namespace anki {

class PassLevelKey;
class Renderer;
class FrustumComponent;
class SceneNode;
class ShaderProgram;
class RenderComponent;

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
		SceneNode& renderableSceneNode,
		U32* subSpatialIndices,
		U subSpatialIndicesCount);

private:
	Renderer* r;

	void setupShaderProg(
		const PassLevelKey& key,
		const FrustumComponent& fr,
		const ShaderProgram& prog,
		RenderComponent& renderable,
		U32* subSpatialIndices,
		U subSpatialIndicesCount,
		F32 flod);
};

} // end namespace anki

#endif
