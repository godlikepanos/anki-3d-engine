// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_LIGHT_H
#define ANKI_SCENE_LIGHT_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Forward.h"
#include "anki/scene/LightComponent.h"
#include "anki/resource/Resource.h"
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

	virtual void onMoveUpdate(MoveComponent& move) = 0;

	virtual void onShapeUpdate(LightComponent& light) = 0;
};

/// Point light
class PointLight: public Light
{
public:
	PointLight(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name);

	/// @name SceneNode virtuals
	/// @{
	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;
	/// @}

public:
	class ShadowData
	{
	public:
#if 0
		ShadowData(SceneNode* node)
		:	m_frustumComps{{
				{node, &m_frustums[0]}, {node, &m_frustums[1]},
				{node, &m_frustums[2]}, {node, &m_frustums[3]},
				{node, &m_frustums[4]}, {node, &m_frustums[5]}}}
		{}

		Array<PerspectiveFrustum, 6> m_frustums;
		Array<FrustumComponent, 6> m_frustumComps;
		Array<Transform, 6> m_localTrfs;
#endif
	};

	Sphere m_sphereW = Sphere(Vec4(0.0), 1.0);
	ShadowData* m_shadowData = nullptr;

	void onMoveUpdate(MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};

/// Spot light
class SpotLight: public Light
{
public:
	SpotLight(SceneGraph* scene);

	ANKI_USE_RESULT Error create(const CString& name);

private:
	PerspectiveFrustum m_frustum;

	void onMoveUpdate(MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};
/// @}

} // end namespace anki

#endif
