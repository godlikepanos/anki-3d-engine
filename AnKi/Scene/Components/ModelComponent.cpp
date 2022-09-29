// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ModelComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ModelResource.h>
#include <AnKi/Resource/ResourceManager.h>

namespace anki {

ANKI_SCENE_COMPONENT_STATICS(ModelComponent)

ModelComponent::ModelComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
{
}

ModelComponent::~ModelComponent()
{
	m_modelPatchMergeKeys.destroy(m_node->getMemoryPool());
}

Error ModelComponent::loadModelResource(CString filename)
{
	m_dirty = true;

	ModelResourcePtr rsrc;
	ANKI_CHECK(m_node->getSceneGraph().getResourceManager().loadResource(filename, rsrc));
	m_model = std::move(rsrc);

	m_modelPatchMergeKeys.destroy(m_node->getMemoryPool());
	m_modelPatchMergeKeys.create(m_node->getMemoryPool(), m_model->getModelPatches().getSize());

	for(U32 i = 0; i < m_model->getModelPatches().getSize(); ++i)
	{
		Array<U64, 2> toHash;
		toHash[0] = i;
		toHash[1] = m_model->getUuid();
		m_modelPatchMergeKeys[i] = computeHash(&toHash[0], sizeof(toHash));
	}

	return Error::kNone;
}

} // end namespace anki
