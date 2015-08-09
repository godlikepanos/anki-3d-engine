// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/SceneNode.h"
#include "anki/scene/Forward.h"
#include "anki/scene/LightComponent.h"
#include "anki/resource/TextureResource.h"
#include "anki/Collision.h"

namespace anki {

/// @addtogroup scene
/// @{

/// Light scene node. It can be spot or point.
class Light: public SceneNode
{
	friend class LightFeedbackComponent;

public:
	Light(SceneGraph* scene);

	~Light();

	ANKI_USE_RESULT Error create(
		const CString& name,
		LightComponent::LightType type,
		CollisionShape* shape);

	ANKI_USE_RESULT Error loadLensFlare(const CString& filename);

protected:
	/// Called when moved
	void onMoveUpdateCommon(MoveComponent& move);

	/// One of the frustums got updated
	void onShapeUpdateCommon(LightComponent& light);

	void frameUpdateCommon();

	virtual void onMoveUpdate(MoveComponent& move) = 0;

	virtual void onShapeUpdate(LightComponent& light) = 0;
};

/// Point light
class PointLight: public Light
{
public:
	PointLight(SceneGraph* scene);
	~PointLight();

	ANKI_USE_RESULT Error create(const CString& name);

	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;

public:
	class ShadowCombo
	{
	public:
		PerspectiveFrustum m_frustum;
		Transform m_localTrf;
	};

	Sphere m_sphereW = Sphere(Vec4(0.0), 1.0);
	DArray<ShadowCombo> m_shadowData;

	void onMoveUpdate(MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};

/// Spot light
class SpotLight: public Light
{
public:
	SpotLight(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name);

	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;

private:
	PerspectiveFrustum m_frustum;

	void onMoveUpdate(MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};
/// @}

} // end namespace anki

