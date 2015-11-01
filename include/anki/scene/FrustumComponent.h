// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Frustum.h>
#include <anki/scene/SpatialComponent.h>
#include <anki/scene/Common.h>
#include <anki/scene/SceneComponent.h>
#include <anki/util/Bitset.h>

namespace anki {

// Forward
class VisibilityTestResults;

/// @addtogroup scene
/// @{

/// Frustum component interface for scene nodes. Useful for nodes that are
/// frustums like cameras and lights
class FrustumComponent: public SceneComponent
{
public:
	struct VisibilityStats
	{
		U32 m_renderablesCount = 0;
		U32 m_lightsCount = 0;
	};

	/// Flags that affect visibility tests.
	enum class VisibilityTestFlag: U8
	{
		TEST_NONE = 0,
		TEST_RENDER_COMPONENTS = 1 << 0,
		TEST_LIGHT_COMPONENTS = 1 << 1,
		TEST_LENS_FLARE_COMPONENTS = 1 << 2,
		TEST_SHADOW_CASTERS = 1 << 3,
		TEST_REFLECTION_PROBES = 1 << 4,
		TEST_REFLECTION_PROXIES = 1 << 5,

		ALL_TESTS =
			TEST_RENDER_COMPONENTS
			| TEST_LIGHT_COMPONENTS
			| TEST_LENS_FLARE_COMPONENTS
			| TEST_SHADOW_CASTERS
			| TEST_REFLECTION_PROBES
			| TEST_REFLECTION_PROXIES
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VisibilityTestFlag, friend)

	/// Pass the frustum here so we can avoid the virtuals
	FrustumComponent(SceneNode* node, Frustum* frustum);

	Frustum& getFrustum()
	{
		return *m_frustum;
	}

	const Frustum& getFrustum() const
	{
		return *m_frustum;
	}

	const Mat4& getProjectionMatrix() const
	{
		return m_pm;
	}

	const Mat4& getViewMatrix() const
	{
		return m_vm;
	}

	const Mat4& getViewProjectionMatrix() const
	{
		return m_vpm;
	}

	/// Parameters used to get the view space position using the depth value
	/// and the NDC xy coordinates.
	/// @code
	/// vec3 fragPos;
	/// fragPos.z = projectionParams.z / (projectionParams.w + depth);
	/// fragPos.xy = projectionParams * NDC * fragPos.z;
	/// @endcode
	const Vec4& getProjectionParameters() const
	{
		return m_projParams;
	}

	/// Get the origin for sorting and visibility tests
	const Vec4& getFrustumOrigin() const
	{
		return m_frustum->getTransform().getOrigin();
	}

	void setVisibilityTestResults(VisibilityTestResults* visible);

	/// Call this after the tests. Before it will point to junk
	VisibilityTestResults& getVisibilityTestResults()
	{
		ANKI_ASSERT(m_visible != nullptr);
		return *m_visible;
	}

	const VisibilityTestResults& getVisibilityTestResults() const
	{
		ANKI_ASSERT(m_visible != nullptr);
		return *m_visible;
	}

	Bool hasVisibilityTestResults() const
	{
		return m_visible != nullptr;
	}

	const VisibilityStats& getLastVisibilityStats() const
	{
		return m_stats;
	}

	/// Call when the shape of the frustum got changed.
	void markShapeForUpdate()
	{
		m_flags.enableBits(SHAPE_MARKED_FOR_UPDATE);
	}

	/// Call when the transformation of the frustum got changed.
	void markTransformForUpdate()
	{
		m_flags.enableBits(TRANSFORM_MARKED_FOR_UPDATE);
	}

	/// Is a spatial inside the frustum?
	Bool insideFrustum(SpatialComponent& sp) const
	{
		return m_frustum->insideFrustum(sp.getSpatialCollisionShape());
	}

	/// Is a collision shape inside the frustum?
	Bool insideFrustum(const CollisionShape& cs) const
	{
		return m_frustum->insideFrustum(cs);
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override;
	/// @}

	void setEnabledVisibilityTests(VisibilityTestFlag bits)
	{
		m_flags.disableBits(VisibilityTestFlag::ALL_TESTS);
		m_flags.enableBits(bits, true);

#if ANKI_ASSERTS_ENABLED
		if(m_flags.bitsEnabled(VisibilityTestFlag::TEST_RENDER_COMPONENTS)
			|| m_flags.bitsEnabled(VisibilityTestFlag::TEST_SHADOW_CASTERS))
		{
			if(m_flags.bitsEnabled(VisibilityTestFlag::TEST_RENDER_COMPONENTS)
				== m_flags.bitsEnabled(VisibilityTestFlag::TEST_SHADOW_CASTERS))
			{
				ANKI_ASSERT(0 && "Cannot have them both");
			}
		}
#endif
	}

	Bool visibilityTestsEnabled(VisibilityTestFlag bits) const
	{
		return m_flags.bitsEnabled(bits);
	}

	Bool anyVisibilityTestEnabled() const
	{
		return m_flags.anyBitsEnabled(VisibilityTestFlag::ALL_TESTS);
	}

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::FRUSTUM;
	}

private:
	enum Flags
	{
		SHAPE_MARKED_FOR_UPDATE = 1 << 6,
		TRANSFORM_MARKED_FOR_UPDATE = 1 << 7,
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Flags, friend)

	Frustum* m_frustum;
	Mat4 m_pm = Mat4::getIdentity(); ///< Projection matrix
	Mat4 m_vm = Mat4::getIdentity(); ///< View matrix
	Mat4 m_vpm = Mat4::getIdentity(); ///< View projection matrix

	Vec4 m_projParams = Vec4(0.0);

	/// Visibility stuff. It's per frame so the pointer is invalid on the next
	/// frame and before any visibility tests are run
	VisibilityTestResults* m_visible = nullptr;
	VisibilityStats m_stats;

	Bitset<U8> m_flags;

	void computeProjectionParams();
};
/// @}

} // end namespace anki

