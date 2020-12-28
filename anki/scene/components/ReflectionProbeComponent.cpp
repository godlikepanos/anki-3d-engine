// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/ReflectionProbeComponent.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/SceneNode.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(ReflectionProbeComponent)

ReflectionProbeComponent::ReflectionProbeComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_markedForRendering(false)
	, m_markedForUpdate(true)
{
}

} // end namespace anki
