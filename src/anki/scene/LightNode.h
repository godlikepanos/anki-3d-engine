// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/Forward.h>
#include <anki/scene/components/LightComponent.h>
#include <anki/resource/TextureResource.h>
#include <anki/Collision.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// Light scene node. It can be spot or point.
class LightNode : public SceneNode
{
public:
	LightNode(SceneGraph* scene, CString name);

	~LightNode();

	ANKI_USE_RESULT Error init(LightComponentType type, CollisionShape* shape);

	ANKI_USE_RESULT Error loadLensFlare(const CString& filename);

protected:
	/// Called when moved
	void onMoveUpdateCommon(const MoveComponent& move);

	/// One of the frustums got updated
	void onShapeUpdateCommon(LightComponent& light);

	void frameUpdateCommon();

	virtual void onMoveUpdate(const MoveComponent& move) = 0;

	virtual void onShapeUpdate(LightComponent& light) = 0;

private:
	class MovedFeedbackComponent;
	class LightChangedFeedbackComponent;
};

/// Point light
class PointLightNode : public LightNode
{
public:
	PointLightNode(SceneGraph* scene, CString name);
	~PointLightNode();

	ANKI_USE_RESULT Error init();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class ShadowCombo
	{
	public:
		PerspectiveFrustum m_frustum;
		Transform m_localTrf = Transform::getIdentity();
	};

	Sphere m_sphereW = Sphere(Vec4(0.0), 1.0);
	DynamicArray<ShadowCombo> m_shadowData;

	void onMoveUpdate(const MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};

/// Spot light
class SpotLightNode : public LightNode
{
public:
	SpotLightNode(SceneGraph* scene, CString name);

	ANKI_USE_RESULT Error init();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	PerspectiveFrustum m_frustum;

	void onMoveUpdate(const MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};
/// @}

} // end namespace anki
