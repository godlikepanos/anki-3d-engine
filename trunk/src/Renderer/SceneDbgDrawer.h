#ifndef SCENE_DBG_DRAWER_H
#define SCENE_DBG_DRAWER_H


class Dbg;
class Camera;
class Light;
class ParticleEmitter;
class SkinNode;


/// This is a drawer for some scene nodes that need debug
class SceneDbgDrawer
{
	public:
		/// Constructor
		SceneDbgDrawer(Dbg& dbg_);

		/// Draw a Camera
		virtual void drawCamera(const Camera& cam) const;

		/// Draw a Light
		virtual void drawLight(const Light& light) const;

		/// Draw a ParticleEmitter
		virtual void drawParticleEmitter(const ParticleEmitter& pe) const;

		/// Draw a skeleton
		virtual void drawSkinNodeSkeleton(const SkinNode& pe) const;

	private:
		Dbg& dbg; ///< The debug stage
};


inline SceneDbgDrawer::SceneDbgDrawer(Dbg& dbg_):
	dbg(dbg_)
{}


#endif
