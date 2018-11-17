// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/SceneNode.h>

namespace anki
{

FrustumComponent::FrustumComponent(SceneNode* node, Frustum* frustum)
	: SceneComponent(CLASS_TYPE)
	, m_node(node)
	, m_frustum(frustum)
	, m_flags(0)
{
	// WARNING: Never touch m_frustum in constructor
	ANKI_ASSERT(node);
	ANKI_ASSERT(frustum);
	markShapeForUpdate();
	markTransformForUpdate();

	setEnabledVisibilityTests(FrustumComponentVisibilityTestFlag::NONE);
}

FrustumComponent::~FrustumComponent()
{
	m_coverageBuff.m_depthMap.destroy(m_node->getAllocator());
}

Error FrustumComponent::update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated)
{
	ANKI_ASSERT(&node == m_node);

	updated = false;
	m_prevViewProjMat = m_viewProjMat;

	if(m_flags.get(SHAPE_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_projMat = m_frustum->calculateProjectionMatrix();
	}

	if(m_flags.get(TRANSFORM_MARKED_FOR_UPDATE))
	{
		updated = true;
		m_viewMat = Mat4(m_frustum->getTransform().getInverse());
	}

	if(updated)
	{
		m_viewProjMat = m_projMat * m_viewMat;
		m_flags.unset(SHAPE_MARKED_FOR_UPDATE | TRANSFORM_MARKED_FOR_UPDATE);
	}

	return Error::NONE;
}

void FrustumComponent::fillCoverageBufferCallback(void* userData, F32* depthValues, U32 width, U32 height)
{
	ANKI_ASSERT(userData && depthValues && width > 0 && height > 0);
	FrustumComponent& self = *static_cast<FrustumComponent*>(userData);

	self.m_coverageBuff.m_depthMap.destroy(self.m_node->getAllocator());
	self.m_coverageBuff.m_depthMap.create(self.m_node->getAllocator(), width * height);
	memcpy(&self.m_coverageBuff.m_depthMap[0], depthValues, self.m_coverageBuff.m_depthMap.getSizeInBytes());

	self.m_coverageBuff.m_depthMapWidth = width;
	self.m_coverageBuff.m_depthMapHeight = height;
}

} // end namespace anki
