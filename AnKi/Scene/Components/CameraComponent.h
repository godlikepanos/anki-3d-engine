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

	const Frustum& getFrustum() const
	{
		return m_frustum;
	}

	Frustum& getFrustum()
	{
		return m_frustum;
	}

	static void fillCoverage(void* userData, F32* depthValues, U32 width, U32 height);

private:
	Frustum m_frustum;
	MoveComponent* m_moveComponent = nullptr;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added);
};
/// @}

} // end namespace anki
