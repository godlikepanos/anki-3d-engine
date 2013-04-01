#ifndef ANKI_SCENE_PATH_H
#define ANKI_SCENE_PATH_H

#include "anki/scene/Common.h"
#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"

namespace anki {

/// XXX
class PathPoint
{
	friend class Path;

public:
	/// @name Accessors
	/// @{
	const Vec3& getPosition() const
	{
		return pos;
	}

	const Quat& getRotation() const
	{
		return rot;
	}

	F32 getDistance() const
	{
		return dist;
	}

	F32 getDistanceFromFirst() const
	{
		return distStart;
	}
	/// @}

private:
	Vec3 pos;
	Quat rot;
	F32 distStart; ///< Distance from the first PathPoint of the path
	F32 dist; ///< Distance from the previous PathPoint in the path list
};

/// XXX
class Path: public SceneNode, public Movable
{
public:
	Path(const char* filename,
		const char* name, SceneGraph* scene, // SceneNode
		U32 movableFlags, Movable* movParent); // Movable

	const SceneVector<PathPoint>& getPoints() const
	{
		return points;
	}

	F32 getDistance() const
	{
		return distance;
	}

private:
	SceneVector<PathPoint> points;
	F32 distance;
};

} // end namespace anki

#endif
