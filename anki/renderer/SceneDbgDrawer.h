#ifndef ANKI_RENDERER_SCENE_DBG_DRAWER_H
#define ANKI_RENDERER_SCENE_DBG_DRAWER_H

#include "anki/util/StdTypes.h"
#include "anki/scene/SceneNode.h"


namespace anki {


class Dbg;
class Octree;
class OctreeNode;


/// This is a drawer for some scene nodes that need debug
class SceneDbgDrawer
{
public:
	enum DebugFlag
	{
		DF_NONE = 0,
		DF_SPATIAL = 1,
		DF_MOVABLE = 2,
		DF_FRUSTUMABLE = 4
	};

	SceneDbgDrawer(Dbg* d)
		: dbg(d), flags(DF_NONE)
	{}

	virtual ~SceneDbgDrawer()
	{}

	/// @name Flag manipulation
	/// @{
	void enableFlag(DebugFlag flag, bool enable = true)
	{
		flags = enable ? flags | flag : flags & ~flag;
	}
	void disableFlag(DebugFlag flag)
	{
		enableFlag(flag, false);
	}
	bool isFlagEnabled(DebugFlag flag) const
	{
		return flags & flag;
	}
	uint getFlagsBitmask() const
	{
		return flags;
	}
	/// @}

	void draw(const SceneNode& node);

	virtual void draw(const Octree& octree) const;

private:
	Dbg* dbg;
	uint flags;

	virtual void draw(const Frustumable& fr) const;

	virtual void draw(const Spatial& sp) const;

	virtual void draw(const OctreeNode& octnode,
		uint depth, const Octree& octree) const;
};


} // end namespace


#endif
