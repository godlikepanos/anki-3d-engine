#ifndef ANKI_COLLISION_COLLISION_SHAPE_H
#define ANKI_COLLISION_COLLISION_SHAPE_H

#include "anki/collision/Forward.h"
#include "anki/collision/CollisionAlgorithms.h"
#include "anki/Math.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Visitor.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Abstract class for collision shapes. It also features a visitor for
/// implementing specific code outside the collision codebase (eg rendering)
class CollisionShape
{
public:
	/// Collision shape type
	enum class Type: U8
	{
		LINE_SEG,
		RAY,
		PLANE,
		SPHERE,
		AABB,
		OBB,
		FRUSTUM,
		COMPOUND
	};

	/// Generic mutable visitor
	class MutableVisitor
	{
	public:
		virtual ~MutableVisitor()
		{}

		virtual void visit(LineSegment&) = 0;
		virtual void visit(Obb&) = 0;
		virtual void visit(Frustum&) = 0;
		virtual void visit(Plane&) = 0;
		virtual void visit(Ray&) = 0;
		virtual void visit(Sphere&) = 0;
		virtual void visit(Aabb&) = 0;
		virtual void visit(CompoundShape&) = 0;
	};

	/// Generic const visitor
	class ConstVisitor
	{
	public:
		virtual ~ConstVisitor()
		{}

		virtual void visit(const LineSegment&) = 0;
		virtual void visit(const Obb&) = 0;
		virtual void visit(const Frustum&) = 0;
		virtual void visit(const Plane&) = 0;
		virtual void visit(const Ray&) = 0;
		virtual void visit(const Sphere&) = 0;
		virtual void visit(const Aabb&) = 0;
		virtual void visit(const CompoundShape&) = 0;
	};

	/// @name Constructors & destructor
	/// @{
	CollisionShape(Type cid)
		: m_cid(cid)
	{}

	virtual ~CollisionShape()
	{}
	/// @}

	/// @name Accessors
	/// @{
	Type getType() const
	{
		return m_cid;
	}
	/// @}

	/// Check for collision
	template<typename T>
	Bool collide(const T& x) const
	{
		return detail::collide(*this, x);
	}

	/// If the collision shape intersects with the plane then the method
	/// returns 0.0, else it returns the distance. If the distance is < 0.0
	/// then the collision shape lies behind the plane and if > 0.0 then
	/// in front of it
	virtual F32 testPlane(const Plane& p) const = 0;

	/// Transform
	virtual void transform(const Transform& trf) = 0;

	/// Get the AABB
	virtual void computeAabb(Aabb&) const = 0;

	/// Visitor accept
	virtual void accept(MutableVisitor&) = 0;
	/// Visitor accept
	virtual void accept(ConstVisitor&) const = 0;

private:
	/// Keep an ID to avoid (in some cases) the visitor and thus the cost of
	/// virtuals
	Type m_cid;
};
/// @}

} // end namespace anki

#endif
