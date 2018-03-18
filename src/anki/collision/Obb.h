// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/ConvexShape.h>
#include <anki/collision/Aabb.h>
#include <anki/Math.h>
#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Object oriented bounding box
class Obb : public ConvexShape
{
public:
	using Base = ConvexShape;

	Obb()
		: Base(CollisionShapeType::OBB)
	{
	}

	Obb(const Obb& b)
		: Base(CollisionShapeType::OBB)
	{
		operator=(b);
	}

	Obb(const Vec4& center, const Mat3x4& rotation, const Vec4& extend)
		: Base(CollisionShapeType::OBB)
		, m_center(center)
		, m_rotation(rotation)
		, m_transposedRotation(rotation)
		, m_extend(extend)
	{
		ANKI_ASSERT(m_center.w() == 0.0f && m_extend.w() == 0.0f);
		m_transposedRotation.transposeRotationPart();
	}

	Obb& operator=(const Obb& b)
	{
		m_center = b.m_center;
		m_rotation = b.m_rotation;
		m_transposedRotation = b.m_transposedRotation;
		m_extend = b.m_extend;
		m_cache = b.m_cache;
		return *this;
	}

	const Vec4& getCenter() const
	{
		return m_center;
	}

	void setCenter(const Vec4& x)
	{
		makeDirty();
		m_center = x;
	}

	const Mat3x4& getRotation() const
	{
		return m_rotation;
	}

	void setRotation(const Mat3x4& x)
	{
		makeDirty();
		m_rotation = x;
		m_transposedRotation = x;
		m_transposedRotation.transposeRotationPart();
	}

	const Vec4& getExtend() const
	{
		return m_extend;
	}

	void setExtend(const Vec4& x)
	{
		makeDirty();
		m_extend = x;
	}

	/// Implements CollisionShape::accept
	void accept(MutableVisitor& v) override
	{
		v.visit(*this);
	}
	/// Implements CollisionShape::accept
	void accept(ConstVisitor& v) const override
	{
		v.visit(*this);
	}

	/// Implements CollisionShape::testPlane
	F32 testPlane(const Plane& p) const override;

	/// Implements CollisionShape::transform
	void transform(const Transform& trf) override
	{
		*this = getTransformed(trf);
	}

	/// Implements CollisionShape::computeAabb
	void computeAabb(Aabb& aabb) const override;

	Obb getTransformed(const Transform& transform) const;

	/// Get a collision shape that includes this and the given. It's not very accurate.
	Obb getCompoundShape(const Obb& b) const;

	/// Calculate from a set of points
	void setFromPointCloud(const void* buff, U count, PtrSize stride, PtrSize buffSize);

	/// Get extreme points in 3D space
	void getExtremePoints(Array<Vec4, 8>& points) const;

	/// Implements ConvexShape::computeSupport
	Vec4 computeSupport(const Vec4& dir) const override;

private:
	Vec4 m_center = Vec4(0.0f);
	Mat3x4 m_rotation = Mat3x4::getIdentity();
	Mat3x4 m_transposedRotation = Mat3x4::getIdentity(); ///< Used for visibility tests
	/// With identity rotation this points to max (front, right, top in our case)
	Vec4 m_extend = Vec4(Vec3(EPSILON), 0.0);

	class
	{
	public:
		mutable Aabb m_aabb;
		mutable Array<Vec4, 8> m_extremePoints;
		mutable Bool8 m_dirtyAabb = true;
		mutable Bool8 m_dirtyExtremePoints = true;
	} m_cache;

	void makeDirty()
	{
		m_cache.m_dirtyAabb = true;
		m_cache.m_dirtyExtremePoints = true;
	}
};

/// @}

} // end namespace anki
