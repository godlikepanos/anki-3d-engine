// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>

#include <AnKi/Scene/Components/BodyComponent.h>
#include <AnKi/Scene/Components/CameraComponent.h>
#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/Components/GlobalIlluminationProbeComponent.h>
#include <AnKi/Scene/Components/JointComponent.h>
#include <AnKi/Scene/Components/LensFlareComponent.h>
#include <AnKi/Scene/Components/LightComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/ParticleEmitter2Component.h>
#include <AnKi/Scene/Components/PlayerControllerComponent.h>
#include <AnKi/Scene/Components/ReflectionProbeComponent.h>
#include <AnKi/Scene/Components/ScriptComponent.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/Components/SkyboxComponent.h>
#include <AnKi/Scene/Components/TriggerComponent.h>
#include <AnKi/Scene/Components/UiComponent.h>
#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Scene/Components/MaterialComponent.h>

namespace anki {

// Specialize newComponent(). Do that first
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	template<> \
	name##Component* SceneNode::newComponent<name##Component>() \
	{ \
		if(hasComponent<name##Component>() && !kSceneComponentSceneNodeCanHaveMany[SceneComponentType::k##name]) \
		{ \
			ANKI_SCENE_LOGE("Can't have many %s components in scene node %s", kSceneComponentTypeName[SceneComponentType::k##name], \
							getName().cstr()); \
			return nullptr; \
		} \
		auto it = SceneGraph::getSingleton().getComponentArrays().get##name##s().emplace(this); \
		it->setArrayIndex(it.getArrayIndex()); \
		newComponentInternal(&(*it)); \
		return &(*it); \
	}
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

SceneNode::SceneNode(CString name)
	: m_uuid(SceneGraph::getSingleton().getNewUuid())
{
	if(name)
	{
		m_name = name;
	}

	// Add the implicit MoveComponent
	newComponent<MoveComponent>();
}

SceneNode::~SceneNode()
{
	for(SceneComponent* comp : m_components)
	{
		comp->onDestroy(*this);

		switch(comp->getType())
		{
#define ANKI_DEFINE_SCENE_COMPONENT(name, weight, sceneNodeCanHaveMany, icon, serializable) \
	case SceneComponentType::k##name: \
		SceneGraph::getSingleton().getComponentArrays().get##name##s().erase(comp->getArrayIndex()); \
		break;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>
		default:
			ANKI_ASSERT(0);
		}
	}
}

void SceneNode::markForDeletion()
{
	visitThisAndChildren([](SceneNode& obj) {
		obj.m_markedForDeletion = true;
		return true;
	});
}

void SceneNode::newComponentInternal(SceneComponent* newc)
{
	m_componentTypeMask |= 1 << SceneComponentTypeMask(newc->getType());

	// Inform all other components that some component was added
	for(SceneComponent* other : m_components)
	{
		other->onOtherComponentRemovedOrAdded(newc, true);
	}

	// Inform the current component about others
	for(SceneComponent* other : m_components)
	{
		newc->onOtherComponentRemovedOrAdded(other, true);
	}

	m_components.emplaceBack(newc);

	// Sort based on update weight
	std::sort(m_components.getBegin(), m_components.getEnd(), [](const SceneComponent* a, const SceneComponent* b) {
		const F32 weightA = SceneComponent::getUpdateOrderWeight(a->getType());
		const F32 weightB = SceneComponent::getUpdateOrderWeight(b->getType());
		if(weightA != weightB)
		{
			return weightA < weightB;
		}
		else
		{
			return a->getType() < b->getType();
		}
	});
}

Bool SceneNode::updateTransform()
{
	const Bool needsUpdate = m_localTransformDirty;
	m_localTransformDirty = false;
	const Bool updatedLastFrame = m_transformUpdatedThisFrame;
	m_transformUpdatedThisFrame = needsUpdate;

	if(needsUpdate || updatedLastFrame)
	{
		m_prevWTrf = m_wtrf;
	}

	// Update world transform
	if(needsUpdate)
	{
		const SceneNode* parent = getParent();

		if(parent == nullptr || m_ignoreParentNodeTransform)
		{
			m_wtrf = m_ltrf;
		}
		else
		{
			m_wtrf = parent->getWorldTransform().combineTransformations(m_ltrf);
		}

		// Make children dirty as well. Don't walk the whole tree because you will re-walk it later
		visitChildrenMaxDepth(1, [](SceneNode& childNode) {
			if(!childNode.m_ignoreParentNodeTransform)
			{
				childNode.m_localTransformDirty = true;
			}
			return true;
		});
	}

	return needsUpdate;
}

void SceneNode::setName(CString name)
{
	const SceneString oldName = getName();
	m_name = name;
	SceneGraph::getSingleton().sceneNodeChangedName(*this, oldName);
}

} // end namespace anki
