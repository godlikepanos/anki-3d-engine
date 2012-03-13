#include "anki/renderer/SceneDbgDrawer.h"
#include "anki/renderer/Dbg.h"
#include "anki/renderer/CollisionDbgDrawer.h"
#include "anki/scene/Octree.h"
#include "anki/scene/Frustumable.h"
#include "anki/scene/Spatial.h"


namespace anki {


//==============================================================================
void SceneDbgDrawer::draw(const SceneNode& node)
{
	if(isFlagEnabled(DF_FRUSTUMABLE) && node.getFrustumable())
	{
		draw(*node.getFrustumable());
	}

	if(isFlagEnabled(DF_SPATIAL) && node.getSpatial())
	{
		draw(*node.getSpatial());
	}
}


//==============================================================================
void SceneDbgDrawer::draw(const Frustumable& fr) const
{
	const Frustum& fs = fr.getFrustum();

	CollisionDbgDrawer coldraw(dbg);
	fs.accept(coldraw);
}


//==============================================================================
void SceneDbgDrawer::draw(const Spatial& x) const
{
	const CollisionShape& cs = x.getSpatialCollisionShape();

	CollisionDbgDrawer coldraw(dbg);
	cs.accept(coldraw);
}


//==============================================================================
void SceneDbgDrawer::draw(const Octree& octree) const
{
	dbg->setColor(Vec3(1.0));
	draw(octree.getRoot(), 0, octree);
}


//==============================================================================
void SceneDbgDrawer::draw(const OctreeNode& octnode, uint depth,
	const Octree& octree) const
{
	Vec3 color = Vec3(1.0 - float(depth) / float(octree.getMaxDepth()));
	dbg->setColor(color);

	CollisionDbgDrawer v(dbg);
	octnode.getAabb().accept(v);

	// Children
	for(uint i = 0; i < 8; ++i)
	{
		if(octnode.getChildren()[i] != NULL)
		{
			draw(*octnode.getChildren()[i], depth + 1, octree);
		}
	}
}


} // end namespace
