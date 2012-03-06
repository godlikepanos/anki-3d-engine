#ifndef ANKI_RENDERER_SCENE_DBG_DRAWER_H
#define ANKI_RENDERER_SCENE_DBG_DRAWER_H

#include "anki/util/StdTypes.h"


namespace anki {


class Frustumable;
class Spatial;
class Octree;
class OctreeNode;

class Dbg;


/// This is a drawer for some scene nodes that need debug
class SceneDbgDrawer
{
public:
	virtual void draw(const Frustumable& fr, Dbg& dbg) const;

	virtual void draw(const Spatial& sp, Dbg& dbg) const;

	virtual void draw(const Octree& octree, Dbg& dbg) const;

	virtual void draw(const OctreeNode& octnode,
		uint depth, const Octree& octree, Dbg& dbg) const;

private:
	virtual void draw(const PerspectiveFrustum& cam, Dbg& dbg) const;
	virtual void draw(const OrthographicFrustum& cam, Dbg& dbg) const;
};


} // end namespace


#endif
