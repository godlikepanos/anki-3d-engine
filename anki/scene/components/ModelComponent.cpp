// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/ModelComponent.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(ModelComponent)

ModelComponent::ModelComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
{
}

ModelComponent::~ModelComponent()
{
}

} // end namespace anki
