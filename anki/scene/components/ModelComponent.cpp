// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/components/ModelComponent.h>
#include <anki/scene/SceneNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/resource/ModelResource.h>
#include <anki/resource/ResourceManager.h>

namespace anki
{

ANKI_SCENE_COMPONENT_STATICS(ModelComponent)

ModelComponent::ModelComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
}

ModelComponent::~ModelComponent()
{
	m_modelPatchMergeKeys.destroy(m_node->getAllocator());
}

Error ModelComponent::loadModelResource(CString filename)
{
	m_dirty = true;

	ModelResourcePtr rsrc;
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(filename, rsrc));
	m_model = rsrc;

	m_modelPatchMergeKeys.destroy(m_node->getAllocator());
	m_modelPatchMergeKeys.create(m_node->getAllocator(), m_model->getModelPatches().getSize());

	for(U32 i = 0; i < m_model->getModelPatches().getSize(); ++i)
	{
		Array<U64, 2> toHash;
		toHash[0] = i;
		toHash[1] = m_model->getUuid();
		m_modelPatchMergeKeys[i] = computeHash(&toHash[0], sizeof(toHash));
	}

	return Error::NONE;
}

} // end namespace anki
