// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/GlobalIlluminationProbeComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(GlobalIlluminationProbeComponent)

GlobalIlluminationProbeComponent::GlobalIlluminationProbeComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_uuid(node->getSceneGraph().getNewUuid())
	, m_markedForRendering(false)
	, m_shapeDirty(true)
{
}

} // end namespace anki
