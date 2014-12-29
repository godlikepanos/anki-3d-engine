// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_FRUSTUM_COMPONENT_H
#define ANKI_SCENE_FRUSTUM_COMPONENT_H

#include "anki/collision/Frustum.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"

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

	/// Pass the frustum here so we can avoid the virtuals
	FrustumComponent(SceneNode* node, Frustum* frustum)
	:	SceneComponent(Type::FRUSTUM, node), 
		m_frustum(frustum),
		m_flags(0)
	{
		// WARNING: Never touch m_frustum in constructor
		ANKI_ASSERT(frustum);
		markShapeForUpdate();
		markTransformForUpdate();
	}

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

	void setVisibilityTestResults(VisibilityTestResults* visible);

	/// Call this after the tests. Before it will point to junk
	VisibilityTestResults& getVisibilityTestResults()
	{
		ANKI_ASSERT(m_visible != nullptr);
		return *m_visible;
	}

	const VisibilityStats& getLastVisibilityStats() const
	{
		return m_stats;
	}

	/// Call when the shape of the frustum got changed.
	void markShapeForUpdate()
	{
		m_flags |= SHAPE_MARKED_FOR_UPDATE;
	}

	/// Call when the transformation of the frustum got changed.
	void markTransformForUpdate()
	{
		m_flags |= TRANSFORM_MARKED_FOR_UPDATE;
	}

	/// Is a spatial inside the frustum?
	Bool insideFrustum(SpatialComponent& sp)
	{
		return m_frustum->insideFrustum(sp.getSpatialCollisionShape());
	}

	/// Is a collision shape inside the frustum?
	Bool insideFrustum(const CollisionShape& cs)
	{
		return m_frustum->insideFrustum(cs);
	}

	/// @name SceneComponent overrides
	/// @{
	ANKI_USE_RESULT Error update(SceneNode&, F32, F32, Bool& updated) override;

	void reset() override
	{
		m_visible = nullptr;
	}
	/// @}

	static constexpr Type getClassType()
	{
		return Type::FRUSTUM;
	}

private:
	enum Flags
	{
		SHAPE_MARKED_FOR_UPDATE = 1 << 0,
		TRANSFORM_MARKED_FOR_UPDATE = 1 << 1
	};

	Frustum* m_frustum;
	Mat4 m_pm = Mat4::getIdentity(); ///< Projection matrix
	Mat4 m_vm = Mat4::getIdentity(); ///< View matrix
	Mat4 m_vpm = Mat4::getIdentity(); ///< View projection matrix

	/// Visibility stuff. It's per frame so the pointer is invalid on the next 
	/// frame and before any visibility tests are run
	VisibilityTestResults* m_visible = nullptr;
	VisibilityStats m_stats;

	U8 m_flags;
};
/// @}

} // end namespace anki

#endif
