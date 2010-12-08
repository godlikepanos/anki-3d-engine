#ifndef SCENE_DBG_DRAWER_H
#define SCENE_DBG_DRAWER_H

#include "Object.h"


class Dbg;
class Camera;
class Light;
class ParticleEmitter;


/// This is a drawer for some scene nodes that need debug
class SceneDbgDrawer: public Object
{
	public:
		/// Constructor
		SceneDbgDrawer(Dbg& dbg_, Object* parent);

		/// Draw a Camera
		virtual void drawCamera(const Camera& cam) const;

		/// Draw a Light
		virtual void drawLight(const Light& light) const;

		/// Draw a ParticleEmitter
		virtual void drawParticleEmitter(const ParticleEmitter& pe) const;

	private:
		Dbg& dbg; ///< The debug stage
};


inline SceneDbgDrawer::SceneDbgDrawer(Dbg& dbg_, Object* parent):
	Object(parent),
	dbg(dbg_)
{}


#endif
