#ifndef ANKI_COLLISION_COLLISION_SHAPE_TRANSFORM_H
#define ANKI_COLLISION_COLLISION_SHAPE_TRANSFORM_H

#include "anki/collision/Forward.h"
#include "anki/math/Forward.h"


namespace anki {


/// Transforms a shape
class CollisionShapeTransform
{
	public:
		/// Generic transform
		static void transform(const Transform& trf, CollisionShape& cs);
};


} // end namespace


#endif
