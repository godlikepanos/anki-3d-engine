// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_COLLISION_OBB_H
#define ANKI_COLLISION_OBB_H

#include "anki/collision/ConvexShape.h"
#include "anki/Math.h"
#include "anki/util/Array.h"

namespace anki {

/// @addtogroup collision
/// @{

/// Object oriented bounding box
class Obb: public ConvexShape
{
public:
	using Base = ConvexShape;

	/// @name Constructors
	/// @{
	Obb();

	Obb(const Obb& b);

	Obb(const Vec4& center, const Mat3x4& rotation, const Vec4& extend);
	/// @}

	/// @name Accessors
	/// @{
	const Vec4& getCenter() const
	{
		return m_center;
	}
	Vec4& getCenter()
	{
		return m_center;
	}
	void setCenter(const Vec4& x)
	{
		m_center = x;
	}

	const Mat3x4& getRotation() const
	{
		return m_rotation;
	}
	Mat3x4& getRotation()
	{
		return m_rotation;
	}
	void setRotation(const Mat3x4& x)
	{
		m_rotation = x;
	}

	const Vec4& getExtend() const
	{
		return m_extend;
	}
	Vec4& getExtend()
	{
		return m_extend;
	}
	void setExtend(const Vec4& x)
	{
		m_extend = x;
	}
	/// @}

	/// @name Operators
	/// @{
	Obb& operator=(const Obb& b)
	{
		m_center = b.m_center;
		m_rotation = b.m_rotation;
		m_extend = b.m_extend;
		return *this;
	}
	/// @}

	/// Implements CollisionShape::accept
	void accept(MutableVisitor& v)
	{
		v.visit(*this);
	}
	/// Implements CollisionShape::accept
	void accept(ConstVisitor& v) const
	{
		v.visit(*this);
	}

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf)
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& aabb) const;

	Obb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. Its not
	/// very accurate
	Obb getCompoundShape(const Obb& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(
		const void* buff, U count, PtrSize stride, PtrSize buffSize);

	/// Get extreme points in 3D space
	void getExtremePoints(Array<Vec4, 8>& points) const;

	/// Implements ConvexShape::computeSupport
	Vec4 computeSupport(const Vec4& dir) const;

public:
	/// @name Data
	/// @{
	Vec4 m_center;
	Mat3x4 m_rotation;
	/// With identity rotation this points to max (front, right, top in
	/// our case)
	Vec4 m_extend;
	/// @}
};

/// @}

} // end namespace anki

#endif
