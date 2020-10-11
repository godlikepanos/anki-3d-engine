// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/Forward.h>
#include <anki/scene/DebugDrawer.h>
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

	ANKI_USE_RESULT Error loadLensFlare(const CString& filename);

protected:
	class MovedFeedbackComponent;
	class LightChangedFeedbackComponent;

	ANKI_USE_RESULT Error initCommon(LightComponentType lightType);

	/// Called when moved
	void onMoveUpdateCommon(const MoveComponent& move);

	void frameUpdateCommon();

	virtual void onMoveUpdate(const MoveComponent& move) = 0;

	virtual void onShapeUpdate(LightComponent& light) = 0;

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);

private:
	DebugDrawer2 m_dbgDrawer;
	TextureResourcePtr m_dbgTex;
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
		Transform m_localTrf = Transform::getIdentity();
	};

	Sphere m_sphereW = Sphere(Vec4(0.0f), 1.0f);
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
	void onMoveUpdate(const MoveComponent& move) override;
	void onShapeUpdate(LightComponent& light) override;
};

/// Directional light (the sun).
class DirectionalLightNode : public SceneNode
{
public:
	DirectionalLightNode(SceneGraph* scene, CString name);

	ANKI_USE_RESULT Error init();

private:
	class FeedbackComponent;

	Aabb m_boundingBox;

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}
};
/// @}

} // end namespace anki
