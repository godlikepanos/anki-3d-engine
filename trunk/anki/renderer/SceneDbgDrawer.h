#ifndef ANKI_RENDERER_SCENE_DBG_DRAWER_H
#define ANKI_RENDERER_SCENE_DBG_DRAWER_H


namespace anki {


class Camera;
class Light;
class ParticleEmitterNode;
class SkinNode;
class PerspectiveCamera;
class OrthographicCamera;

class Dbg;


/// This is a drawer for some scene nodes that need debug
class SceneDbgDrawer
{
	public:
		/// Constructor
		SceneDbgDrawer(Dbg& dbg_)
		:	dbg(dbg_)
		{}

		/// Draw a Camera
		virtual void drawCamera(const Camera& cam) const;

		/// Draw a Light
		virtual void drawLight(const Light& light) const;

		/// Draw a ParticleEmitterNode
		virtual void drawParticleEmitter(const ParticleEmitterNode& pe) const;

		/// Draw a skeleton
		virtual void drawSkinNodeSkeleton(const SkinNode& pe) const;

	private:
		Dbg& dbg; ///< The debug stage

		virtual void drawPerspectiveCamera(const PerspectiveCamera& cam) const;
		virtual void drawOrthographicCamera(
			const OrthographicCamera& cam) const;
};


} // end namespace


#endif
