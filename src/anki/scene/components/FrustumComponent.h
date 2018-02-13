// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/collision/Frustum.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/Common.h>
#include <anki/scene/components/SceneComponent.h>
#include <anki/util/BitMask.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Flags that affect visibility tests.
/// WARNING!!!!!!!!!!: If you change this remember to change the FrustumComponent::Flags as well
enum class FrustumComponentVisibilityTestFlag : U16
{
	NONE = 0,
	RENDER_COMPONENTS = 1 << 0,
	LIGHT_COMPONENTS = 1 << 1,
	LENS_FLARE_COMPONENTS = 1 << 2,
	SHADOW_CASTERS = 1 << 3,
	REFLECTION_PROBES = 1 << 4,
	REFLECTION_PROXIES = 1 << 5,
	OCCLUDERS = 1 << 6,
	DECALS = 1 << 7,
	EARLY_Z = 1 << 8,

	ALL_TESTS = RENDER_COMPONENTS | LIGHT_COMPONENTS | LENS_FLARE_COMPONENTS | SHADOW_CASTERS | REFLECTION_PROBES
				| REFLECTION_PROXIES | DECALS | EARLY_Z
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(FrustumComponentVisibilityTestFlag, inline)

/// Frustum component interface for scene nodes. Useful for nodes that are frustums like cameras and lights.
class FrustumComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::FRUSTUM;

	struct VisibilityStats
	{
		U32 m_renderablesCount = 0;
		U32 m_lightsCount = 0;
	};

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

	/// Get the origin for sorting and visibility tests
	const Vec4& getFrustumOrigin() const
	{
		return m_frustum->getTransform().getOrigin();
	}

	/// Call when the shape of the frustum got changed.
	void markShapeForUpdate()
	{
		m_flags.set(SHAPE_MARKED_FOR_UPDATE);
	}

	/// Call when the transformation of the frustum got changed.
	void markTransformForUpdate()
	{
		m_flags.set(TRANSFORM_MARKED_FOR_UPDATE);
	}

	/// Is a spatial inside the frustum?
	Bool insideFrustum(const SpatialComponent& sp) const
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
	ANKI_USE_RESULT Error update(SceneNode&, Second, Second, Bool& updated) override;
	/// @}

	void setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag bits)
	{
		m_flags.unset(FrustumComponentVisibilityTestFlag::ALL_TESTS);
		m_flags.set(bits, true);

#if ANKI_ASSERTS_ENABLED
		if(m_flags.get(FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS)
			|| m_flags.get(FrustumComponentVisibilityTestFlag::SHADOW_CASTERS))
		{
			if(m_flags.get(FrustumComponentVisibilityTestFlag::RENDER_COMPONENTS)
				== m_flags.get(FrustumComponentVisibilityTestFlag::SHADOW_CASTERS))
			{
				ANKI_ASSERT(0 && "Cannot have them both");
			}
		}
#endif
	}

	Bool visibilityTestsEnabled(FrustumComponentVisibilityTestFlag bits) const
	{
		return m_flags.get(bits);
	}

	Bool anyVisibilityTestEnabled() const
	{
		return m_flags.getAny(FrustumComponentVisibilityTestFlag::ALL_TESTS);
	}

private:
	enum Flags
	{
		SHAPE_MARKED_FOR_UPDATE = 1 << 9,
		TRANSFORM_MARKED_FOR_UPDATE = 1 << 10,
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Flags, friend)

	Frustum* m_frustum;
	Mat4 m_pm = Mat4::getIdentity(); ///< Projection matrix
	Mat4 m_vm = Mat4::getIdentity(); ///< View matrix
	Mat4 m_vpm = Mat4::getIdentity(); ///< View projection matrix

	BitMask<U16> m_flags;
};
/// @}

} // end namespace anki
