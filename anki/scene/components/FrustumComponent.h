// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/util/BitMask.h>
#include <anki/util/WeakArray.h>
#include <anki/collision/Obb.h>
#include <anki/collision/ConvexHullShape.h>
#include <anki/collision/Plane.h>
#include <anki/shaders/include/ClusteredShadingFunctions.h>
#include <anki/resource/Common.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Flags that affect visibility tests.
enum class FrustumComponentVisibilityTestFlag : U32
{
	NONE = 0,
	RENDER_COMPONENTS = 1 << 0,
	LIGHT_COMPONENTS = 1 << 1,
	LENS_FLARE_COMPONENTS = 1 << 2,
	SHADOW_CASTERS = 1 << 3, ///< Render components that cast shadow
	POINT_LIGHT_SHADOWS_ENABLED = 1 << 4,
	SPOT_LIGHT_SHADOWS_ENABLED = 1 << 5,
	DIRECTIONAL_LIGHT_SHADOWS_ALL_CASCADES = 1 << 6,
	DIRECTIONAL_LIGHT_SHADOWS_1_CASCADE = 1 << 7,
	REFLECTION_PROBES = 1 << 8,
	REFLECTION_PROXIES = 1 << 9,
	OCCLUDERS = 1 << 10,
	DECALS = 1 << 11,
	FOG_DENSITY_COMPONENTS = 1 << 12,
	GLOBAL_ILLUMINATION_PROBES = 1 << 13,
	EARLY_Z = 1 << 14,
	GENERIC_COMPUTE_JOB_COMPONENTS = 1 << 15,
	RAY_TRACING_SHADOWS = 1 << 16,
	RAY_TRACING_GI = 1 << 17,
	RAY_TRACING_REFLECTIONS = 1 << 18,
	RAY_TRACING_PATH_TRACING = 1 << 19,

	ALL = RENDER_COMPONENTS | LIGHT_COMPONENTS | LENS_FLARE_COMPONENTS | SHADOW_CASTERS | POINT_LIGHT_SHADOWS_ENABLED
		  | SPOT_LIGHT_SHADOWS_ENABLED | DIRECTIONAL_LIGHT_SHADOWS_ALL_CASCADES | DIRECTIONAL_LIGHT_SHADOWS_1_CASCADE
		  | REFLECTION_PROBES | REFLECTION_PROXIES | OCCLUDERS | DECALS | FOG_DENSITY_COMPONENTS
		  | GLOBAL_ILLUMINATION_PROBES | EARLY_Z | GENERIC_COMPUTE_JOB_COMPONENTS | RAY_TRACING_SHADOWS | RAY_TRACING_GI
		  | RAY_TRACING_REFLECTIONS | RAY_TRACING_PATH_TRACING,

	ALL_SHADOWS_ENABLED =
		POINT_LIGHT_SHADOWS_ENABLED | SPOT_LIGHT_SHADOWS_ENABLED | DIRECTIONAL_LIGHT_SHADOWS_ALL_CASCADES,

	ALL_RAY_TRACING = RAY_TRACING_SHADOWS | RAY_TRACING_GI | RAY_TRACING_REFLECTIONS | RAY_TRACING_PATH_TRACING
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FrustumComponentVisibilityTestFlag)

/// Frustum component. Useful for nodes that take part in visibility tests like cameras and lights.
class FrustumComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::FRUSTUM;

	FrustumComponent(SceneNode* node, FrustumType frustumType);

	~FrustumComponent();

	SceneNode& getSceneNode()
	{
		return *m_node;
	}

	FrustumType getFrustumType() const
	{
		return m_frustumType;
	}

	void setPerspective(F32 near, F32 far, F32 fovX, F32 fovY)
	{
		ANKI_ASSERT(near > 0.0f && far > 0.0f && near < far);
		ANKI_ASSERT(fovX > 0.0f && fovY > 0.0f && fovX < PI && fovY < PI);
		ANKI_ASSERT(m_frustumType == FrustumType::PERSPECTIVE);
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
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
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
		ANKI_ASSERT(m_frustumType == FrustumType::PERSPECTIVE);
		m_shapeMarkedForUpdate = true;
		m_perspective.m_fovX = fovx;
	}

	F32 getFovX() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::PERSPECTIVE);
		return m_perspective.m_fovX;
	}

	void setFovY(F32 fovy)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::PERSPECTIVE);
		m_shapeMarkedForUpdate = true;
		m_perspective.m_fovY = fovy;
	}

	F32 getFovY() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::PERSPECTIVE);
		return m_perspective.m_fovY;
	}

	F32 getLeft() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		return m_ortho.m_left;
	}

	void setLeft(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		m_ortho.m_left = value;
	}

	F32 getRight() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		return m_ortho.m_right;
	}

	void setRight(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		m_ortho.m_right = value;
	}

	F32 getTop() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		return m_ortho.m_top;
	}

	void setTop(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		m_ortho.m_top = value;
	}

	F32 getBottom() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		return m_ortho.m_bottom;
	}

	void setBottom(F32 value)
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		m_ortho.m_bottom = value;
	}

	const SceneNode& getSceneNode() const
	{
		return *m_node;
	}

	const Transform& getTransform() const
	{
		return m_trf;
	}

	void setTransform(const Transform& trf)
	{
		m_trf = trf;
		m_trfMarkedForUpdate = true;
	}

	const Mat4& getProjectionMatrix() const
	{
		return m_projMat;
	}

	const Mat4& getViewMatrix() const
	{
		return m_viewMat;
	}

	const Mat4& getViewProjectionMatrix() const
	{
		return m_viewProjMat;
	}

	const Mat4& getPreviousViewProjectionMatrix() const
	{
		return m_prevViewProjMat;
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

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		ANKI_ASSERT(&node == m_node);
		updated = updateInternal();
		return Error::NONE;
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
												  getEffectiveShadowDistance(), getCascadeCount());
	}

	const ConvexHullShape& getPerspectiveBoundingShape() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::PERSPECTIVE);
		return m_perspective.m_hull;
	}

	const Obb& getOrthographicBoundingShape() const
	{
		ANKI_ASSERT(m_frustumType == FrustumType::ORTHOGRAPHIC);
		return m_ortho.m_obbW;
	}

	const Array<Plane, U32(FrustumPlaneType::COUNT)>& getViewPlanes() const
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

	FrustumType m_frustumType = FrustumType::COUNT;

	union
	{
		Perspective m_perspective;
		Ortho m_ortho;
		Common m_common;
	};

	// View planes
	Array<Plane, U32(FrustumPlaneType::COUNT)> m_viewPlanesL;
	Array<Plane, U32(FrustumPlaneType::COUNT)> m_viewPlanesW;

	Transform m_trf = Transform::getIdentity();
	Mat4 m_projMat = Mat4::getIdentity(); ///< Projection matrix
	Mat4 m_viewMat = Mat4::getIdentity(); ///< View matrix
	Mat4 m_viewProjMat = Mat4::getIdentity(); ///< View projection matrix
	Mat4 m_prevViewProjMat = Mat4::getIdentity();

	/// How far to render shadows for this frustum. If negative it's the m_frustum's far.
	F32 m_effectiveShadowDistance = -1.0f;

	/// Defines the the rate of the cascade distances
	F32 m_shadowCascadesDistancePower = 1.0f;

	Array<F32, MAX_LOD_COUNT> m_maxLodDistances = {};

	class
	{
	public:
		DynamicArray<F32> m_depthMap;
		U32 m_depthMapWidth = 0;
		U32 m_depthMapHeight = 0;
	} m_coverageBuff; ///< Coverage buffer for extra visibility tests.

	FrustumComponentVisibilityTestFlag m_flags = FrustumComponentVisibilityTestFlag::NONE;
	Bool m_shapeMarkedForUpdate = true;
	Bool m_trfMarkedForUpdate = true;

	Bool updateInternal();

	U32 getCascadeCount() const
	{
		return !!(m_flags & FrustumComponentVisibilityTestFlag::DIRECTIONAL_LIGHT_SHADOWS_ALL_CASCADES)
				   ? MAX_SHADOW_CASCADES
				   : 1;
	}
};
/// @}

} // end namespace anki
