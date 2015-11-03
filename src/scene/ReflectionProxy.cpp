// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProxy.h>
#include <anki/scene/ReflectionProxyComponent.h>
#include <anki/scene/MoveComponent.h>
#include <anki/scene/SpatialComponent.h>

namespace anki {

//==============================================================================
// ReflectionProxyMoveFeedbackComponent                                        =
//==============================================================================

/// Feedback component
class ReflectionProxyMoveFeedbackComponent: public SceneComponent
{
public:
	ReflectionProxyMoveFeedbackComponent(SceneNode* node)
		: SceneComponent(SceneComponent::Type::NONE, node)
	{}

	Error update(SceneNode& node, F32, F32, Bool& updated) final
	{
		updated = false;

		MoveComponent& move = node.getComponent<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			// Move updated
			ReflectionProxy& dnode = static_cast<ReflectionProxy&>(node);
			dnode.onMoveUpdate(move);
		}

		return ErrorCode::NONE;
	}
};

//==============================================================================
// ReflectionProxy                                                             =
//==============================================================================

//==============================================================================
Error ReflectionProxy::create(const CString& name, F32 width, F32 height,
	F32 maxDistance)
{
	ANKI_CHECK(SceneNode::create(name));

	// Move component first
	SceneComponent* comp = getSceneAllocator().newInstance<MoveComponent>(this);
	addComponent(comp, true);

	// Feedback component
	comp = getSceneAllocator().
		newInstance<ReflectionProxyMoveFeedbackComponent>(this);
	addComponent(comp, true);

	// Proxy component
	ReflectionProxyComponent* proxyc =
		getSceneAllocator().newInstance<ReflectionProxyComponent>(this);
	m_quadLSpace[0] = Vec4(-width / 2.0, -height / 2.0, 0.0, 0.0);
	m_quadLSpace[1] = Vec4(width / 2.0, -height / 2.0, 0.0, 0.0);
	m_quadLSpace[2] = Vec4(width / 2.0, height / 2.0, 0.0, 0.0);
	m_quadLSpace[3] = Vec4(-width / 2.0, height / 2.0, 0.0, 0.0);
	proxyc->setVertex(0, m_quadLSpace[0]);
	proxyc->setVertex(1, m_quadLSpace[1]);
	proxyc->setVertex(2, m_quadLSpace[2]);
	proxyc->setVertex(3, m_quadLSpace[3]);

	addComponent(proxyc, true);

	// Spatial component
	m_boxLSpace.setCenter(Vec4(0.0, 0.0, maxDistance / 2.0, 0.0));
	m_boxLSpace.setRotation(Mat3x4::getIdentity());
	m_boxLSpace.setExtend(Vec4(width / 2.0 + maxDistance,
		height / 2.0 + maxDistance, maxDistance / 2.0, 0.0));

	m_boxWSpace = m_boxLSpace;

	comp = getSceneAllocator().newInstance<SpatialComponent>(this,
		&m_boxWSpace);
	addComponent(comp, true);

	return ErrorCode::NONE;
}

//==============================================================================
void ReflectionProxy::onMoveUpdate(const MoveComponent& move)
{
	const Transform& trf = move.getWorldTransform();

	// Update proxy comp
	ReflectionProxyComponent& proxyc = getComponent<ReflectionProxyComponent>();
	for(U i = 0; i < 4; ++i)
	{
		proxyc.setVertex(i, trf.transform(m_quadLSpace[i]));
	}

	// Update spatial
	m_boxWSpace = m_boxLSpace;
	m_boxWSpace.transform(trf);
	SpatialComponent& spatial = getComponent<SpatialComponent>();
	spatial.markForUpdate();
}

} // end namespace anki

