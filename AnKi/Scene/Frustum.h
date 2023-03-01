// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Collision/Obb.h>
#include <AnKi/Collision/ConvexHullShape.h>
#include <AnKi/Collision/Plane.h>

namespace anki {

/// @addtogroup scene
/// @{

/// A helper class that represents a frustum.
class Frustum
{
public:
	Frustum();

	~Frustum();

	void init(FrustumType type, HeapMemoryPool* pool);

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
		return m_projMat;
	}

	const Mat3x4& getViewMatrix() const
	{
		return m_viewMat;
	}

	const Mat4& getViewProjectionMatrix() const
	{
		return m_viewProjMat;
	}

	/// @param nMinusOneFrame The number of the previous frame. If 0 means the previous frame, 1 means the
	///                       previous-previous frame.
	const Mat4& getPreviousViewProjectionMatrix(U32 nMinusOneFrame = 0) const
	{
		return m_prevViewProjMats[nMinusOneFrame];
	}

	/// @see getPreviousViewProjectionMatrix.
	const Mat3x4& getPreviousViewMatrix(U32 nMinusOneFrame = 0) const
	{
		return m_prevViewMats[nMinusOneFrame];
	}

	/// @see getPreviousViewProjectionMatrix.
	const Mat4& getPreviousProjectionMatrix(U32 nMinusOneFrame = 0) const
	{
		return m_prevProjMats[nMinusOneFrame];
	}

	/// Check if a shape is inside the frustum.
	template<typename T>
	Bool insideFrustum(const T& t) const
	{
		for(const Plane& plane : m_viewPlanesW)
		{
			if(testPlane(plane, t) < 0.0f)
			{
				return false;
			}
		}

		return true;
	}

	Bool hasCoverageBuffer() const
	{
		return m_depthMap.getSize() > 0;
	}

	void getCoverageBufferInfo(ConstWeakArray<F32>& depthBuff, U32& width, U32& height) const
	{
		if(m_depthMap.getSize() > 0)
		{
			depthBuff = ConstWeakArray<F32>(&m_depthMap[0], m_depthMap.getSize());
			width = m_depthMapWidth;
			height = m_depthMapHeight;
			ANKI_ASSERT(width > 0 && height > 0);
		}
		else
		{
			depthBuff = ConstWeakArray<F32>();
			width = height = 0;
		}
	}

	void setCoverageBuffer(F32* depths, U32 width, U32 height);

	void setShadowCascadeDistance(U32 cascade, F32 distance)
	{
		m_shadowCascadeDistances[cascade] = distance;
		m_miscDirty = true;
	}

	F32 getShadowCascadeDistance(U32 cascade) const
	{
		return m_shadowCascadeDistances[cascade];
	}

	[[nodiscard]] U32 getShadowCascadeCount() const
	{
		return m_shadowCascadeCount;
	}

	void setShadowCascadeCount(U32 count)
	{
		ANKI_ASSERT(count <= kMaxShadowCascades);
		m_shadowCascadeCount = U8(count);
		m_miscDirty = true;
	}

	const ConvexHullShape& getPerspectiveBoundingShapeWorldSpace() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		return m_perspective.m_hull;
	}

	const Obb& getOrthographicBoundingShapeWorldSpace() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		return m_ortho.m_obbW;
	}

	const Array<Plane, U32(FrustumPlaneType::kCount)>& getViewPlanes() const
	{
		return m_viewPlanesW;
	}

	/// Set the lod distance. It doesn't interact with the far plane.
	void setLodDistance(U32 lod, F32 maxDistance)
	{
		m_maxLodDistances[lod] = maxDistance;
		m_miscDirty = true;
	}

	void setLodDistances(const Array<F32, kMaxLodCount>& distances)
	{
		for(U i = 0; i < m_maxLodDistances.getSize(); ++i)
		{
			ANKI_ASSERT(distances[i] > m_common.m_near && distances[i] <= m_common.m_far);
			m_maxLodDistances[i] = distances[i];
		}
	}

	/// See setLodDistance.
	F32 getLodDistance(U32 lod) const
	{
		ANKI_ASSERT(m_maxLodDistances[lod] > 0.0f);
		return m_maxLodDistances[lod];
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

	F32 getEarlyZDistance() const
	{
		return m_earlyZDistance;
	}

	void setEarlyZDistance(F32 dist)
	{
		ANKI_ASSERT(dist >= 0.0f);
		m_earlyZDistance = dist;
	}

	Bool getUpdatedThisFrame() const
	{
		return m_updatedThisFrame;
	}

	Bool update();

	/// Get the precalculated rotations of each of the 6 frustums of an omnidirectional source (eg a point light).
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
		Array<Vec4, 5> m_edgesW;
		Array<Vec4, 4> m_edgesL; ///< Don't need the eye point.
		ConvexHullShape m_hull;
	};

	class Ortho : public Common
	{
	public:
		F32 m_left;
		F32 m_right;
		F32 m_top;
		F32 m_bottom;
		Obb m_obbL;
		Obb m_obbW; ///< Including shape
	};

	static constexpr F32 kDefaultNear = 0.1f;
	static constexpr F32 kDefaultFar = 100.0f;
	static constexpr F32 kDefaultFovAngle = toRad(45.0f);

	HeapMemoryPool* m_pool = nullptr;

	union
	{
		Perspective m_perspective;
		Ortho m_ortho;
		Common m_common;
	};

	// View planes
	Array<Plane, U32(FrustumPlaneType::kCount)> m_viewPlanesL;
	Array<Plane, U32(FrustumPlaneType::kCount)> m_viewPlanesW;

	Transform m_worldTransform = Transform::getIdentity();

	Mat4 m_projMat = Mat4::getIdentity(); ///< Projection matrix
	Mat3x4 m_viewMat = Mat3x4::getIdentity(); ///< View matrix
	Mat4 m_viewProjMat = Mat4::getIdentity(); ///< View projection matrix

	static constexpr U32 kPrevMatrixHistory = 2;
	Array<Mat3x4, kPrevMatrixHistory> m_prevViewMats = {Mat3x4::getIdentity(), Mat3x4::getIdentity()};
	Array<Mat4, kPrevMatrixHistory> m_prevProjMats = {Mat4::getIdentity(), Mat4::getIdentity()};
	Array<Mat4, kPrevMatrixHistory> m_prevViewProjMats = {Mat4::getIdentity(), Mat4::getIdentity()};

	Array<F32, kMaxShadowCascades> m_shadowCascadeDistances = {};
	Array<F32, kMaxLodCount - 1> m_maxLodDistances = {};

	F32 m_earlyZDistance = 0.0f;

	// Coverage buffer stuff
	DynamicArray<F32> m_depthMap;
	U32 m_depthMapWidth = 0;
	U32 m_depthMapHeight = 0;

	FrustumType m_frustumType = FrustumType::kCount;

	U8 m_shadowCascadeCount = 0;

	Bool m_shapeDirty : 1 = true;
	Bool m_miscDirty : 1 = true;
	Bool m_worldTransformDirty : 1 = true;
	Bool m_updatedThisFrame : 1 = true;

	static Array<Mat3x4, 6> m_omnidirectionalRotations;
};
/// @}

} // end namespace anki
