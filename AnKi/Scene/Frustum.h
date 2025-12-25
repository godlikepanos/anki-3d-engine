// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Collision/Obb.h>
#include <AnKi/Collision/ConvexHullShape.h>
#include <AnKi/Collision/Plane.h>

namespace anki {

// A helper class that represents a frustum.
class Frustum
{
public:
	Frustum();

	~Frustum();

	void init(FrustumType type);

	void setPerspective(F32 near, F32 far, F32 fovX, F32 fovY)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		setNear(near);
		setFar(far);
		setFovX(fovX);
		setFovY(fovY);
	}

	void setOrthographic(F32 near, F32 far, F32 right, F32 left, F32 top, F32 bottom)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		setNear(near);
		setFar(far);
		setLeft(left);
		setRight(right);
		setTop(top);
		setBottom(bottom);
	}

	FrustumType getFrustumType() const
	{
		ANKI_ASSERT(m_frustumType != FrustumType::kCount);
		return m_frustumType;
	}

	void setNear(F32 near)
	{
		ANKI_ASSERT(near > kEpsilonf);
		m_common.m_near = near;
		m_shapeDirty = true;
	}

	F32 getNear() const
	{
		return m_common.m_near;
	}

	void setFar(F32 far)
	{
		ANKI_ASSERT(far > kEpsilonf);
		m_common.m_far = far;
		m_shapeDirty = true;
	}

	F32 getFar() const
	{
		return m_common.m_far;
	}

	void setFovX(F32 fovx)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective && fovx > 0.0f && fovx < kPi);
		m_shapeDirty = true;
		m_perspective.m_fovX = fovx;
	}

	F32 getFovX() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		return m_perspective.m_fovX;
	}

	void setFovY(F32 fovy)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective && fovy > 0.0f && fovy < kPi);
		m_shapeDirty = true;
		m_perspective.m_fovY = fovy;
	}

	F32 getFovY() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		return m_perspective.m_fovY;
	}

	F32 getLeft() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		return m_ortho.m_left;
	}

	void setLeft(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		m_shapeDirty = true;
		m_ortho.m_left = value;
	}

	F32 getRight() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		return m_ortho.m_right;
	}

	void setRight(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		m_shapeDirty = true;
		m_ortho.m_right = value;
	}

	F32 getTop() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		return m_ortho.m_top;
	}

	void setTop(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		m_shapeDirty = true;
		m_ortho.m_top = value;
	}

	F32 getBottom() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		return m_ortho.m_bottom;
	}

	void setBottom(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		m_shapeDirty = true;
		m_ortho.m_bottom = value;
	}

	const Mat4& getProjectionMatrix() const
	{
		if(isDirty())
		{
			ANKI_SCENE_LOGW("update() hasn't been called");
		}
		return m_projMat;
	}

	const Mat3x4& getViewMatrix() const
	{
		if(isDirty())
		{
			ANKI_SCENE_LOGW("update() hasn't been called");
		}
		return m_viewMat;
	}

	const Mat4& getViewProjectionMatrix() const
	{
		if(isDirty())
		{
			ANKI_SCENE_LOGW("update() hasn't been called");
		}
		return m_viewProjMat;
	}

	void setWorldTransform(const Transform& worldTransform)
	{
		m_worldTransform = worldTransform;
		m_worldTransformDirty = true;
	}

	const Transform& getWorldTransform() const
	{
		return m_worldTransform;
	}

	Bool update();

	// Get the precalculated rotations of each of the 6 frustums of an omnidirectional source (eg a point light).
	static const Array<Mat3x4, 6>& getOmnidirectionalFrustumRotations()
	{
		return m_omnidirectionalRotations;
	}

private:
	class Common
	{
	public:
		F32 m_near;
		F32 m_far;
	};

	class Perspective : public Common
	{
	public:
		F32 m_fovX;
		F32 m_fovY;
	};

	class Ortho : public Common
	{
	public:
		F32 m_left;
		F32 m_right;
		F32 m_top;
		F32 m_bottom;
	};

	static constexpr F32 kDefaultNear = 0.1f;
	static constexpr F32 kDefaultFar = 100.0f;
	static constexpr F32 kDefaultFovAngle = toRad(45.0f);

	union
	{
		Perspective m_perspective;
		Ortho m_ortho;
		Common m_common;
	};

	Transform m_worldTransform = Transform::getIdentity();

	Mat4 m_projMat = Mat4::getIdentity(); ///< Projection matrix
	Mat3x4 m_viewMat = Mat3x4::getIdentity(); ///< View matrix
	Mat4 m_viewProjMat = Mat4::getIdentity(); ///< View projection matrix

	FrustumType m_frustumType = FrustumType::kCount;

	Bool m_shapeDirty : 1 = true;
	Bool m_worldTransformDirty : 1 = true;

	static Array<Mat3x4, 6> m_omnidirectionalRotations;

	Bool isDirty() const
	{
		return m_shapeDirty || m_worldTransformDirty;
	}
};

} // end namespace anki
