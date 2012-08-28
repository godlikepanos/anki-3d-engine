#ifndef ANKI_COLLISION_COLLISION_SHAPE
#define ANKI_COLLISION_COLLISION_SHAPE

#include "anki/collision/Forward.h"
#include "anki/math/Forward.h"
#include "anki/util/StdTypes.h"

namespace anki {

/// @addtogroup Collision
/// @{

/// Abstract class for collision shapes. It also features a visitor for
/// implementing specific code outside the collision codebase (eg rendering)
class CollisionShape
{
public:
	/// Collision shape type
	enum CollisionShapeType
	{
		CST_LINE_SEG,
		CST_RAY,
		CST_PLANE,
		CST_SPHERE,
		CST_AABB,
		CST_OBB,
		CST_FRUSTUM
	};

	/// Generic mutable visitor
	struct MutableVisitor
	{
		virtual ~MutableVisitor()
		{}

		virtual void visit(LineSegment&) = 0;
		virtual void visit(Obb&) = 0;
		virtual void visit(Frustum&) = 0;
		virtual void visit(Plane&) = 0;
		virtual void visit(Ray&) = 0;
		virtual void visit(Sphere&) = 0;
		virtual void visit(Aabb&) = 0;
	};

	/// Generic const visitor
	struct ConstVisitor
	{
		virtual ~ConstVisitor()
		{}

		virtual void visit(const LineSegment&) = 0;
		virtual void visit(const Obb&) = 0;
		virtual void visit(const Frustum&) = 0;
		virtual void visit(const Plane&) = 0;
		virtual void visit(const Ray&) = 0;
		virtual void visit(const Sphere&) = 0;
		virtual void visit(const Aabb&) = 0;
	};

	/// @name Constructors & destructor
	/// @{
	CollisionShape(CollisionShapeType cid_)
		: cid(cid_)
	{}
	/// @}

	/// @name Accessors
	/// @{
	CollisionShapeType getCollisionShapeType() const
	{
		return cid;
	}
	/// @}

	/// If the collision shape intersects with the plane then the method
	/// returns 0.0, else it returns the distance. If the distance is < 0.0
	/// then the collision shape lies behind the plane and if > 0.0 then
	/// in front of it
	virtual F32 testPlane(const Plane& p) const = 0;

	/// Transform
	virtual void transform(const Transform& trf) = 0;

	/// Get the AABB
	virtual void toAabb(Aabb&) const = 0;

	/// Visitor accept
	virtual void accept(MutableVisitor&) = 0;
	/// Visitor accept
	virtual void accept(ConstVisitor&) const = 0;

private:
	/// Keep an ID to avoid (in some cases) the visitor and thus the cost of
	/// virtuals
	CollisionShapeType cid;
};
/// @}

} // end namespace

#endif
