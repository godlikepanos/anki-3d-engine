// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>

namespace anki {

MeshComponent::MeshComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
}

MeshComponent::~MeshComponent()
{
}

MeshComponent& MeshComponent::setMeshFilename(CString fname)
{
	MeshResourcePtr newRsrc;
	const Error err = ResourceManager::getSingleton().loadResource(fname, newRsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load resource: %s", fname.cstr());
	}
	else
	{
		m_resource = newRsrc;
		m_resourceDirty = true;
	}

	return *this;
}

CString MeshComponent::getMeshFilename() const
{
	return (m_resource) ? m_resource->getFilename() : "*Error*";
}

void MeshComponent::update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_resourceDirty;
	m_resourceDirty = false;
}

} // end namespace anki
