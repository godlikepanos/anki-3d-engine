// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/ReflectionProxy.h>
#include <anki/scene/ReflectionProxyComponent.h>
#include <anki/scene/MoveComponent.h>

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
Error ReflectionProxy::create(const CString& name, const Vec4& p0,
	const Vec4& p1, const Vec4& p2, const Vec4& p3)
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
	m_quadLSpace[0] = p0;
	m_quadLSpace[1] = p1;
	m_quadLSpace[2] = p2;
	m_quadLSpace[3] = p3;
	proxyc->setVertex(0, p0);
	proxyc->setVertex(1, p1);
	proxyc->setVertex(2, p2);
	proxyc->setVertex(3, p3);

	addComponent(proxyc, true);

	return ErrorCode::NONE;
}

//==============================================================================
void ReflectionProxy::onMoveUpdate(const MoveComponent& move)
{
	const Transform& trf = move.getWorldTransform();
	ReflectionProxyComponent& proxyc = getComponent<ReflectionProxyComponent>();
	for(U i = 0; i < 4; ++i)
	{
		proxyc.setVertex(i, trf.transform(m_quadLSpace[i]));
	}
}

} // end namespace anki

