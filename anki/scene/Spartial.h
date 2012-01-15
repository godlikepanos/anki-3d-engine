#ifndef ANKI_SCENE_SPARTIAL_H
#define ANKI_SCENE_SPARTIAL_H

#include "anki/collision/Collision.h"


namespace anki {


/// @addtogroup Scene
/// @{

/// Spartial "interface" for scene nodes
///
/// It indicates scene nodes that need to be placed in the scene's octree and
/// they participate in the visibility tests
class Spartial
{
public:
	/// Pass the collision shape here so we can avoid the virtuals
	Spartial(CollisionShape* cs_)
		: cs(cs_)
	{}

	const CollisionShape& getCollisionShape() const
	{
		return *cs;
	}
	CollisionShape& getCollisionShape()
	{
		return *cs;
	}

private:
	CollisionShape* cs;
};
/// @}


} // namespace anki


#endif
