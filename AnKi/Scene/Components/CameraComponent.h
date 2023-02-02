// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Frustum.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Perspective camera component.
class CameraComponent : public SceneComponent
{
	ANKI_SCENE_COMPONENT(CameraComponent)

public:
	CameraComponent(SceneNode* node);

	~CameraComponent();

	void setNear(F32 near)
	{
		if(ANKI_SCENE_ASSERT(near > 0.0f))
		{
			m_frustum.setNear(near);
		}
	}

	F32 getNear() const
	{
		return m_frustum.getNear();
	}

	void setFar(F32 far)
	{
		if(ANKI_SCENE_ASSERT(far > 0.0f))
		{
			m_frustum.setFar(far);
		}
	}

	F32 getFar() const
	{
		return m_frustum.getFar();
	}

	void setFovX(F32 fovx)
	{
		if(ANKI_SCENE_ASSERT(fovx > 0.0f && fovx < kPi))
		{
			m_frustum.setFovX(fovx);
		}
	}

	F32 getFovX() const
	{
		return m_frustum.getFovX();
	}

	void setFovY(F32 fovy)
	{
		if(ANKI_SCENE_ASSERT(fovy > 0.0f && fovy < kPi))
		{
			m_frustum.setFovY(fovy);
		}
	}

	F32 getFovY() const
	{
		return m_frustum.getFovY();
	}

	void setPerspective(F32 near, F32 far, F32 fovx, F32 fovy)
	{
		m_frustum.setPerspective(near, far, fovx, fovy);
	}

	void setShadowCascadeDistance(U32 cascade, F32 distance)
	{
		if(ANKI_SCENE_ASSERT(cascade < m_frustum.getShadowCascadeCount()))
		{
			m_frustum.setShadowCascadeDistance(cascade, distance);
		}
	}

	F32 getShadowCascadeDistance(U32 cascade) const
	{
		if(ANKI_SCENE_ASSERT(cascade < m_frustum.getShadowCascadeCount()))
		{
			return m_frustum.getShadowCascadeDistance(cascade);
		}
		else
		{
			return 0.0f;
		}
	}

	ANKI_INTERNAL const Frustum& getFrustum() const
	{
		return m_frustum;
	}

	ANKI_INTERNAL Frustum& getFrustum()
	{
		return m_frustum;
	}

	ANKI_INTERNAL Bool getHasExtendedFrustum() const
	{
		return m_usesExtendedFrustum;
	}

	ANKI_INTERNAL const Frustum& getExtendedFrustum() const
	{
		ANKI_ASSERT(m_usesExtendedFrustum);
		return m_extendedFrustum;
	}

	ANKI_INTERNAL Frustum& getExtendedFrustum()
	{
		ANKI_ASSERT(m_usesExtendedFrustum);
		return m_extendedFrustum;
	}

private:
	Frustum m_frustum;
	Frustum m_extendedFrustum; ///< For ray tracing.

	Bool m_usesExtendedFrustum : 1 = false;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	Transform computeExtendedFrustumTransform(const Transform& cameraTransform) const;
};
/// @}

} // end namespace anki
