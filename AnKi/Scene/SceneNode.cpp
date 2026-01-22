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
		if(hasComponent<name##Component>() && !kSceneComponentTypeInfos[SceneComponentType::k##name].m_sceneNodeCanHaveMany) \
		{ \
			ANKI_SCENE_LOGE("Can't have many %s components in scene node %s", kSceneComponentTypeInfos[SceneComponentType::k##name].m_name, \
							getName().cstr()); \
			return nullptr; \
		} \
		SceneComponentInitInfo compInit; \
		compInit.m_node = this; \
		compInit.m_componentUuid = SceneGraph::getSingleton().m_scenes[m_sceneIndex].getNewNodeUuid(); \
		compInit.m_sceneUuid = m_sceneUuid; \
		auto it = SceneGraph::getSingleton().m_componentArrays.get##name##s().emplace(compInit); \
		it->setArrayIndex(it.getArrayIndex()); \
		addComponent(&(*it)); \
		return &(*it); \
	}
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

SceneNode::SceneNode(const SceneNodeInitInfo& inf)
	: m_nodeUuid(inf.m_nodeUuid)
	, m_sceneUuid(inf.m_sceneUuid)
	, m_sceneIndex(U8(inf.m_sceneIndex))
{
	ANKI_ASSERT(m_nodeUuid > 0 && m_nodeUuid == inf.m_nodeUuid);
	ANKI_ASSERT(m_sceneUuid > 0 && m_sceneUuid == inf.m_sceneUuid);
	ANKI_ASSERT(inf.m_sceneIndex < kMaxU32 && m_sceneIndex == inf.m_sceneIndex);

	if(inf.m_name)
	{
		m_name = inf.m_name;
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
		return FunctorContinue::kContinue;
	});
}

void SceneNode::addComponent(SceneComponent* newc)
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
			return FunctorContinue::kContinue;
		});
	}

	return needsUpdate;
}

void SceneNode::setName(CString name)
{
	const SceneString oldName = getName();
	m_name = name;
	SceneGraph::getSingleton().sceneNodeChangedNameDeferred(*this, oldName);
}

Error SceneNode::serializeCommon(SceneSerializer& serializer, SerializeCommonArgs& args)
{
	ANKI_ASSERT(m_serialize);

	// Trf
	Vec3 origin = m_ltrf.getOrigin().xyz;
	ANKI_SERIALIZE(origin, 1);
	m_ltrf.setOrigin(origin);

	Mat3 rotation = m_ltrf.getRotation().getRotationPart();
	ANKI_SERIALIZE(rotation, 1);
	m_ltrf.setRotation(rotation);

	Vec3 scale = m_ltrf.getScale().xyz;
	ANKI_SERIALIZE(scale, 1);
	m_ltrf.setScale(scale);

	// Components
	U32 componentCount = 0;
	SceneDynamicArray<U32> componentUuids;
	if(serializer.isInWriteMode())
	{
		for(SceneComponent* comp : m_components)
		{
			if(comp->getSerialization())
			{
				++componentCount;

				componentUuids.emplaceBack(comp->getComponentUuid());

				args.m_write.m_serializableComponentMask[comp->getType()].setBit(comp->getArrayIndex());
				++args.m_write.m_componentsToBeSerializedCount[comp->getType()];
			}
		}
	}

	ANKI_SERIALIZE(componentCount, 1);

	if(serializer.isInReadMode())
	{
		componentUuids.resize(componentCount, 0);
	}

	ANKI_SERIALIZE(componentUuids, 1);

	if(serializer.isInReadMode())
	{
		for(U32 uuid : componentUuids)
		{
			args.m_read.m_componentUuidToNode.emplace(uuid, this);
		}
	}

	// Parent
	SceneNode* parent = getParent();
	U32 parentUuid = (parent) ? parent->m_nodeUuid : 0;
	ANKI_SERIALIZE(parentUuid, 1);

	if(serializer.isInReadMode() && parentUuid > 0)
	{
		auto it = args.m_read.m_nodeUuidToNode.find(parentUuid);
		if(it == args.m_read.m_nodeUuidToNode.getEnd())
		{
			ANKI_SCENE_LOGE("Failed to find parent UUID: %u", parentUuid);
			return Error::kUserData;
		}

		SceneNode* parent = *it;
		parent->addChild(this);
	}

	return Error::kNone;
}

void SceneNode::setParent(SceneNode* parent)
{
	if(parent)
	{
		if(!ANKI_EXPECT(parent->m_sceneIndex == m_sceneIndex))
		{
			ANKI_SCENE_LOGE("Can't make node %s parent of %s. The don't belong in the same scene", parent->getName().cstr(), getName().cstr());
			return;
		}
	}

	SceneGraph::getSingleton().setSceneNodeParentDeferred(this, parent);
}

} // end namespace anki
