#ifndef ANKI_COLLISION_TO_AABB_H
#define ANKI_COLLISION_TO_AABB_H

#include "anki/collision/Forward.h"
#include "anki/collision/CollisionShape.h"
#include "anki/collision/Aabb.h"


namespace anki {


/// @addtogroup Collision
/// @{

/// XXX
class ToAbb: public CollisionShape::ConstVisitor
{
public:
	void visit(LineSegment&);
	void visit(Obb&);
	void visit(Frustum&);
	void visit(Plane&);
	void visit(Ray&);
	void visit(Sphere&);
	void visit(Aabb&);

	const Aabb& getAabb() const
	{
		return aabb;
	}

private:
	Aabb aabb;
};
/// @}


} // end namespace anki


#endif
