// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Common.h>
#include <anki/Math.h>
#include <anki/util/Array.h>

namespace anki
{

/// @addtogroup collision
/// @{

/// Object oriented bounding box.
class Obb
{
public:
	static constexpr CollisionShapeType CLASS_TYPE = CollisionShapeType::OBB;

	/// Will not initialize any memory, nothing.
	Obb()
	{
	}

	Obb(const Obb& b)
	{
		operator=(b);
	}

	Obb(const Vec4& center, const Mat3x4& rotation, const Vec4& extend)
		: m_center(center)
		, m_extend(extend)
		, m_rotation(rotation)
	{
		check();
	}

	/// Set from point cloud.
	Obb(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize)
	{
		setFromPointCloud(pointBuffer, pointCount, pointStride, buffSize);
	}

	Obb& operator=(const Obb& b)
	{
		m_center = b.m_center;
		m_rotation = b.m_rotation;
		m_extend = b.m_extend;
		return *this;
	}

	const Vec4& getCenter() const
	{
		check();
		return m_center;
	}

	void setCenter(const Vec4& x)
	{
		m_center = x;
	}

	const Mat3x4& getRotation() const
	{
		check();
		return m_rotation;
	}

	void setRotation(const Mat3x4& x)
	{
		m_rotation = x;
	}

	const Vec4& getExtend() const
	{
		check();
		return m_extend;
	}

	void setExtend(const Vec4& x)
	{
		m_extend = x;
	}

	Obb getTransformed(const Transform& trf) const
	{
		check();
		Obb out;
		out.m_extend = m_extend * trf.getScale();
		out.m_center = trf.transform(m_center);
		out.m_rotation = trf.getRotation().combineTransformations(m_rotation);
		return out;
	}

	/// Get a collision shape that includes this and the given. It's not very accurate.
	Obb getCompoundShape(const Obb& b) const;

	/// Calculate from a set of points.
	void setFromPointCloud(const Vec3* pointBuffer, U pointCount, PtrSize pointStride, PtrSize buffSize);

	/// Get extreme points in 3D space
	void getExtremePoints(Array<Vec4, 8>& points) const;

	/// Compute the GJK support.
	ANKI_USE_RESULT Vec4 computeSupport(const Vec4& dir) const;

private:
	Vec4 m_center
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	Vec4 m_extend /// With identity rotation this points to max (front, right, top in our case)
#if ANKI_ENABLE_ASSERTS
		= Vec4(MAX_F32)
#endif
		;

	Mat3x4 m_rotation
#if ANKI_ENABLE_ASSERTS
		= Mat3x4(MAX_F32)
#endif
		;

	void check() const
	{
		ANKI_ASSERT(m_center != Vec4(MAX_F32));
		ANKI_ASSERT(m_extend != Vec4(MAX_F32));
		ANKI_ASSERT(m_rotation != Mat3x4(MAX_F32));
		ANKI_ASSERT(m_center.w() == 0.0f && m_extend.w() == 0.0f);
	}
};

/// @}

} // end namespace anki
