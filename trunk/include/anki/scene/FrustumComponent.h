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

/// @addtogroup Scene
/// @{

/// Frustum component interface for scene nodes. Useful for nodes that are 
/// frustums like cameras and lights
class FrustumComponent: public SceneComponent
{
public:
	/// @name Constructors
	/// @{

	/// Pass the frustum here so we can avoid the virtuals
	FrustumComponent(SceneNode* node, Frustum* frustum)
		: SceneComponent(FRUSTUM_COMPONENT, node), m_frustum(frustum)
	{
		// WARNING: Never touch m_frustum in constructor
		ANKI_ASSERT(frustum);
		markForUpdate();
	}
	/// @}

	/// @name Accessors
	/// @{
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
	void setProjectionMatrix(const Mat4& m)
	{
		m_pm = m;
	}

	const Mat4& getViewMatrix() const
	{
		return m_vm;
	}
	void setViewMatrix(const Mat4& m)
	{
		m_vm = m;
	}

	const Mat4& getViewProjectionMatrix() const
	{
		return m_vpm;
	}
	void setViewProjectionMatrix(const Mat4& m)
	{
		m_vpm = m;
	}

	/// Get the origin for sorting and visibility tests
	const Vec4& getFrustumOrigin() const
	{
		return getFrustum().getTransform().getOrigin();
	}

	void setVisibilityTestResults(VisibilityTestResults* visible)
	{
		ANKI_ASSERT(m_visible == nullptr);
		m_visible = visible;
	}
	/// Call this after the tests. Before it will point to junk
	VisibilityTestResults& getVisibilityTestResults()
	{
		ANKI_ASSERT(m_visible != nullptr);
		return *m_visible;
	}
	/// @}

	void markForUpdate()
	{
		m_markedForUpdate = true;
	}

	/// Is a spatial inside the frustum?
	Bool insideFrustum(SpatialComponent& sp)
	{
		return getFrustum().insideFrustum(sp.getSpatialCollisionShape());
	}

	/// Is a collision shape inside the frustum?
	Bool insideFrustum(const CollisionShape& cs)
	{
		return getFrustum().insideFrustum(cs);
	}

	/// Override SceneComponent::update
	Bool update(SceneNode&, F32, F32, UpdateType updateType) override
	{
		if(updateType == ASYNC_UPDATE)
		{
			Bool out = m_markedForUpdate;
			m_markedForUpdate = false;
			return out;
		}
		else
		{
			return false;
		}
	}

	/// Override SceneComponent::reset
	void reset() override
	{
		m_visible = nullptr;
	}

	static constexpr Type getClassType()
	{
		return FRUSTUM_COMPONENT;
	}

private:
	Frustum* m_frustum;
	Mat4 m_pm = Mat4::getIdentity(); ///< Projection matrix
	Mat4 m_vm = Mat4::getIdentity(); ///< View matrix
	Mat4 m_vpm = Mat4::getIdentity(); ///< View projection matrix

	/// Visibility stuff. It's per frame so the pointer is invalid on the next 
	/// frame and before any visibility tests are run
	VisibilityTestResults* m_visible = nullptr;

	Bool8 m_markedForUpdate;
};
/// @}

} // end namespace anki

#endif
