// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Components/LightComponent.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Light scene node. It can be spot or point.
class LightNode : public SceneNode
{
public:
	LightNode(SceneGraph* scene, CString name);

	~LightNode();

protected:
	class OnMovedFeedbackComponent;
	class OnLightShapeUpdatedFeedbackComponent;

	/// Called when moved
	void onMoveUpdateCommon(const MoveComponent& move);

	void frameUpdateCommon();

	virtual void onMoved(const MoveComponent& move) = 0;

	virtual void onLightShapeUpdated(LightComponent& light) = 0;
};

/// Point light
class PointLightNode : public LightNode
{
public:
	PointLightNode(SceneGraph* scene, CString name);
	~PointLightNode();

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class ShadowCombo
	{
	public:
		Transform m_localTrf = Transform::getIdentity();
	};

	DynamicArray<ShadowCombo> m_shadowData;

	void onMoved(const MoveComponent& move) override;
	void onLightShapeUpdated(LightComponent& light) override;
};

/// Spot light
class SpotLightNode : public LightNode
{
public:
	SpotLightNode(SceneGraph* scene, CString name);

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;

private:
	class OnFrustumUpdatedFeedbackComponent;

	void onMoved(const MoveComponent& move) override;
	void onLightShapeUpdated(LightComponent& light) override;
	void onFrustumUpdated(FrustumComponent& frc);
};

/// Directional light (the sun).
class DirectionalLightNode : public SceneNode
{
public:
	DirectionalLightNode(SceneGraph* scene, CString name);

private:
	class FeedbackComponent;

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData)
	{
		// TODO
	}
};
/// @}

} // end namespace anki
