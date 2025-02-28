// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
		if(ANKI_EXPECT(near > 0.0f))
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
		if(ANKI_EXPECT(far > 0.0f))
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
		if(ANKI_EXPECT(fovx > 0.0f && fovx < kPi))
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
		if(ANKI_EXPECT(fovy > 0.0f && fovy < kPi))
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

	ANKI_INTERNAL const Frustum& getFrustum() const
	{
		return m_frustum;
	}

	ANKI_INTERNAL Frustum& getFrustum()
	{
		return m_frustum;
	}

private:
	Frustum m_frustum;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;
};
/// @}

} // end namespace anki
