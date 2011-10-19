#ifndef ANKI_COLLISION_PLANE_H
#define ANKI_COLLISION_PLANE_H

#include "anki/collision/CollisionShape.h"
#include "anki/math/Math.h"


namespace anki {


/// @addtogroup Collision
/// @{

/// Plane collision shape
class Plane: public CollisionShape
{
	public:
		/// @name Constructors
		/// @{

		/// Default constructor
		Plane()
		:	CollisionShape(CST_PLANE)
		{}

		/// Copy constructor
		Plane(const Plane& b);

		/// Constructor
		Plane(const Vec3& normal_, float offset_);

		/// @see setFrom3Points
		Plane(const Vec3& p0, const Vec3& p1, const Vec3& p2)
		:	CollisionShape(CST_PLANE)
		{
			setFrom3Points(p0, p1, p2);
		}

		/// @see setFromPlaneEquation
		Plane(float a, float b, float c, float d)
		:	CollisionShape(CST_PLANE)
		{
			setFromPlaneEquation(a, b, c, d);
		}
		/// @}

		/// @name Accessors
		/// @{
		const Vec3& getNormal() const
		{
			return normal;
		}
		Vec3& getNormal()
		{
			return normal;
		}
		void setNormal(const Vec3& x)
		{
			normal = x;
		}

		float getOffset() const
		{
			return offset;
		}
		float& getOffset()
		{
			return offset;
		}
		void setOffset(const float x)
		{
			offset = x;
		}
		/// @}

		/// Implements CollisionShape::accept
		void accept(MutableVisitor& v)
		{
			v.visit(*this);
		}
		/// Implements CollisionShape::accept
		void accept(ConstVisitor& v)
		{
			v.visit(*this);
		}

		/// Overrides CollisionShape::testPlane
		float testPlane(const Plane& p) const
		{
			return PlaneTests::test(p, *this);
		}

		/// Overrides CollisionShape::transform
		void transform(const Transform& trf)
		{
			*this = getTransformed(trf);
		}

		/// Return the transformed
		Plane getTransformed(const Transform& trf) const;

		/// It gives the distance between a point and a plane. if returns >0
		/// then the point lies in front of the plane, if <0 then it is behind
		/// and if =0 then it is co-planar
		float test(const Vec3& point) const
		{
			return normal.dot(point) - offset;
		}

		/// Get the distance from a point to this plane
		float getDistance(const Vec3& point) const
		{
			return fabs(test(point));
		}

		/// Returns the perpedicular point of a given point in this plane.
		/// Plane's normal and returned-point are perpedicular
		Vec3 getClosestPoint(const Vec3& point) const
		{
			return point - normal * test(point);
		}

		/// Test a CollisionShape
		template<typename T>
		float testShape(const T& x) const
		{
			return PlaneTests::test(*this, x);
		}

	private:
		/// @name Data
		/// @{
		Vec3 normal;
		float offset;
		/// @}

		/// Set the plane from 3 points
		void setFrom3Points(const Vec3& p0, const Vec3& p1, const Vec3& p2);

		/// Set from plane equation is ax+by+cz+d
		void setFromPlaneEquation(float a, float b, float c, float d);
};
/// @}


} // end namespace


#endif
