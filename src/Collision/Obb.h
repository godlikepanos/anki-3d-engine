#ifndef OBB_H
#define OBB_H

#include <boost/array.hpp>
#include "CollisionShape.h"
#include "Math.h"


/// Object oriented bounding box
class Obb: public CollisionShape
{
	public:
		/// @name Constructors
		/// @{
		Obb(): CollisionShape(CST_OBB) {}
		Obb(const Obb& b);
		Obb(const Vec3& center, const Mat3& rotation, const Vec3& extends);
		/// @}

		/// @name Accessors
		/// @{
		const Vec3& getCenter() const {return center;}
		Vec3& getCenter() {return center;}
		void setCenter(const Vec3& c) {center = c;}

		const Mat3& getRotation() const {return rotation;}
		Mat3& getRotation() {return rotation;}
		void setCenter(const Mat3& r) {rotation = r;}

		const Vec3& getExtend() const {return extends;}
		Vec3& getExtend() {return extends;}
		void setExtend(const Vec3& e) {extends = e;}
		/// @}

		Obb getTransformed(const Transform& transform) const;

		/// Get a collision shape that includes this and the given. Its not very accurate
		Obb getCompoundShape(const Obb& b) const;

		/// @see CollisionShape::testPlane
		float testPlane(const Plane& plane) const;

		/// Calculate from a set of points
		template<typename Container>
		void set(const Container& container);

	public:
		/// @name Data
		/// @{
		Vec3 center;
		Mat3 rotation;
		Vec3 extends; ///< With identity rotation this points to max (front, right, top in our case)
		/// @}

		/// Get extreme points in 3D space
		void getExtremePoints(boost::array<Vec3, 8>& points) const;
};


inline Obb::Obb(const Obb& b):
	CollisionShape(CST_OBB),
	center(b.center),
	rotation(b.rotation),
	extends(b.extends)
{}


inline Obb::Obb(const Vec3& center_, const Mat3& rotation_, const Vec3& extends_):
	CollisionShape(CST_OBB),
	center(center_),
	rotation(rotation_),
	extends(extends_)
{}


#include "Obb.inl.h"


#endif
