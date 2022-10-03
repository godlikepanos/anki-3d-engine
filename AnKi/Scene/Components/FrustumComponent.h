// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Util/BitMask.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Collision/Obb.h>
#include <AnKi/Collision/ConvexHullShape.h>
#include <AnKi/Collision/Plane.h>
#include <AnKi/Resource/Common.h>
#include <AnKi/Shaders/Include/ClusteredShadingFunctions.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Flags that affect visibility tests.
enum class FrustumComponentVisibilityTestFlag : U32
{
	kNone = 0,
	kRenderComponents = 1 << 0,
	kShadowCasterRenderComponents = 1 << 1, ///< Render components that cast shadow
	kLights = 1 << 2,
	kLensFlares = 1 << 3,
	kReflectionProbes = 1 << 4,
	kOccluders = 1 << 5,
	kDecals = 1 << 6,
	kFogDensityVolumes = 1 << 7,
	kGlobalIlluminationProbes = 1 << 8,
	kGenericComputeJobs = 1 << 9,
	kRayTracingShadows = 1 << 10,
	kRayTracingGi = 1 << 11,
	kRayTracingReflections = 1 << 12,
	kRayTracingPathTracing = 1 << 13,
	kUi = 1 << 14,
	kSkybox = 1 << 15,
	kEarlyZ = 1 << 16,
	kPointLightShadowsEnabled = 1 << 17,
	kSpotLightShadowsEnabled = 1 << 18,

	kAll = kRenderComponents | kShadowCasterRenderComponents | kLights | kLensFlares | kReflectionProbes | kOccluders
		   | kDecals | kFogDensityVolumes | kGlobalIlluminationProbes | kGenericComputeJobs | kRayTracingShadows
		   | kRayTracingGi | kRayTracingReflections | kRayTracingPathTracing | kUi | kSkybox | kEarlyZ
		   | kPointLightShadowsEnabled | kSpotLightShadowsEnabled,

	kAllRayTracing = kRayTracingShadows | kRayTracingGi | kRayTracingReflections | kRayTracingPathTracing,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FrustumComponentVisibilityTestFlag)

/// Frustum component. Useful for nodes that take part in visibility tests like cameras and lights.
class FrustumComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(FrustumComponent)

public:
	FrustumComponent(SceneNode* node);

	~FrustumComponent();

	SceneNode& getSceneNode()
	{
		return *m_node;
	}

	const SceneNode& getSceneNode() const
	{
		return *m_node;
	}

	void setFrustumType(FrustumType type)
	{
		ANKI_ASSERT(type >= FrustumType::kFirst && type < FrustumType::kCount);
		m_frustumType = type;
		if(m_frustumType == FrustumType::kPerspective)
		{
			setPerspective(0.1f, 100.0f, toRad(45.0f), toRad(45.0f));
		}
		else
		{
			setOrthographic(0.1f, 100.0f, 5.0f, -5.0f, 5.0f, -5.0f);
		}
	}

	FrustumType getFrustumType() const
	{
		ANKI_ASSERT(m_frustumType != FrustumType::kCount);
		return m_frustumType;
	}

	void setPerspective(F32 near, F32 far, F32 fovX, F32 fovY)
	{
		ANKI_ASSERT(near > 0.0f && far > 0.0f && near < far);
		ANKI_ASSERT(fovX > 0.0f && fovY > 0.0f && fovX < kPi && fovY < kPi);
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		m_perspective.m_near = near;
		m_perspective.m_far = far;
		m_perspective.m_fovX = fovX;
		m_perspective.m_fovY = fovY;
		m_shapeMarkedForUpdate = true;
	}

	void setOrthographic(F32 near, F32 far, F32 right, F32 left, F32 top, F32 bottom)
	{
		ANKI_ASSERT(near > 0.0f && far > 0.0f && near < far);
		ANKI_ASSERT(right > left && top > bottom);
		ANKI_ASSERT(m_frustumType == FrustumType::kOrthographic);
		m_ortho.m_near = near;
		m_ortho.m_far = far;
		m_ortho.m_right = right;
		m_ortho.m_left = left;
		m_ortho.m_top = top;
		m_ortho.m_bottom = bottom;
		m_shapeMarkedForUpdate = true;
	}

	void setNear(F32 near)
	{
		m_common.m_near = near;
		m_shapeMarkedForUpdate = true;
	}

	F32 getNear() const
	{
		return m_common.m_near;
	}

	void setFar(F32 far)
	{
		m_common.m_far = far;
		m_shapeMarkedForUpdate = true;
	}

	F32 getFar() const
	{
		return m_common.m_far;
	}

	void setFovX(F32 fovx)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		m_shapeMarkedForUpdate = true;
		m_perspective.m_fovX = fovx;
	}

	F32 getFovX() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		return m_perspective.m_fovX;
	}

	void setFovY(F32 fovy)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::kPerspective);
		m_shapeMarkedForUpdate = true;
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
		m_ortho.m_bottom = value;
	}

	const Transform& getWorldTransform() const
	{
		return m_trf;
	}

	void setWorldTransform(const Transform& trf)
	{
		m_trf = trf;
		m_trfMarkedForUpdate = true;
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

	Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated) override
	{
		ANKI_ASSERT(info.m_node == m_node);
		updated = updateInternal();
		return Error::kNone;
	}

	void setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag bits);

	FrustumComponentVisibilityTestFlag getEnabledVisibilityTests() const
	{
		return m_flags;
	}

	/// The type is FillCoverageBufferCallback.
	static void fillCoverageBufferCallback(void* userData, F32* depthValues, U32 width, U32 height);

	Bool hasCoverageBuffer() const
	{
		return m_coverageBuff.m_depthMap.getSize() > 0;
	}

	void getCoverageBufferInfo(ConstWeakArray<F32>& depthBuff, U32& width, U32& height) const
	{
		if(m_coverageBuff.m_depthMap.getSize() > 0)
		{
			depthBuff = ConstWeakArray<F32>(&m_coverageBuff.m_depthMap[0], m_coverageBuff.m_depthMap.getSize());
			width = m_coverageBuff.m_depthMapWidth;
			height = m_coverageBuff.m_depthMapHeight;
		}
		else
		{
			depthBuff = ConstWeakArray<F32>();
			width = height = 0;
		}
	}

	/// Set how far to render shadows for this frustum or set to negative if you want to use the m_frustun's far.
	void setEffectiveShadowDistance(F32 distance)
	{
		m_effectiveShadowDistance = distance;
	}

	/// How far to render shadows for this frustum.
	F32 getEffectiveShadowDistance() const
	{
		const F32 distance = (m_effectiveShadowDistance <= 0.0f) ? m_common.m_far : m_effectiveShadowDistance;
		ANKI_ASSERT(distance > m_common.m_near && distance <= m_common.m_far);
		return distance;
	}

	/// See computeShadowCascadeDistance()
	void setShadowCascadesDistancePower(F32 power)
	{
		ANKI_ASSERT(power >= 1.0f);
		m_shadowCascadesDistancePower = power;
	}

	/// See computeShadowCascadeDistance()
	F32 getShadowCascadesDistancePower() const
	{
		return m_shadowCascadesDistancePower;
	}

	F32 computeShadowCascadeDistance(U32 cascadeIdx) const
	{
		return anki::computeShadowCascadeDistance(cascadeIdx, m_shadowCascadesDistancePower,
												  getEffectiveShadowDistance(), getShadowCascadeCount());
	}

	[[nodiscard]] U32 getShadowCascadeCount() const
	{
		return m_shadowCascadeCount;
	}

	void setShadowCascadeCount(U32 count)
	{
		m_shadowCascadeCount = U8(min(count, kMaxShadowCascades));
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
		ANKI_ASSERT(maxDistance > 0.0f);
		m_maxLodDistances[lod] = maxDistance;
	}

	/// See setLodDistance.
	F32 getLodDistance(U32 lod) const
	{
		ANKI_ASSERT(m_maxLodDistances[lod] > 0.0f);
		return m_maxLodDistances[lod];
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

	SceneNode* m_node;

	FrustumType m_frustumType = FrustumType::kCount;

	union
	{
		Perspective m_perspective;
		Ortho m_ortho;
		Common m_common;
	};

	// View planes
	Array<Plane, U32(FrustumPlaneType::kCount)> m_viewPlanesL;
	Array<Plane, U32(FrustumPlaneType::kCount)> m_viewPlanesW;

	Transform m_trf = Transform::getIdentity();
	Mat4 m_projMat = Mat4::getIdentity(); ///< Projection matrix
	Mat3x4 m_viewMat = Mat3x4::getIdentity(); ///< View matrix
	Mat4 m_viewProjMat = Mat4::getIdentity(); ///< View projection matrix

	static constexpr U32 kPrevMatrixHistory = 2;
	Array<Mat3x4, kPrevMatrixHistory> m_prevViewMats = {Mat3x4::getIdentity(), Mat3x4::getIdentity()};
	Array<Mat4, kPrevMatrixHistory> m_prevProjMats = {Mat4::getIdentity(), Mat4::getIdentity()};
	Array<Mat4, kPrevMatrixHistory> m_prevViewProjMats = {Mat4::getIdentity(), Mat4::getIdentity()};

	/// How far to render shadows for this frustum. If negative it's the m_frustum's far.
	F32 m_effectiveShadowDistance = -1.0f;

	/// Defines the the rate of the cascade distances
	F32 m_shadowCascadesDistancePower = 1.0f;

	Array<F32, kMaxLodCount - 1> m_maxLodDistances = {};

	U8 m_shadowCascadeCount = 0;

	class
	{
	public:
		DynamicArray<F32> m_depthMap;
		U32 m_depthMapWidth = 0;
		U32 m_depthMapHeight = 0;
	} m_coverageBuff; ///< Coverage buffer for extra visibility tests.

	FrustumComponentVisibilityTestFlag m_flags = FrustumComponentVisibilityTestFlag::kNone;
	Bool m_shapeMarkedForUpdate : 1;
	Bool m_trfMarkedForUpdate : 1;

	Bool updateInternal();
};
/// @}

} // end namespace anki
