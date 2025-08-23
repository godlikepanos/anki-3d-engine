// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Holds geometry information.
class MeshComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(MeshComponent)

public:
	MeshComponent(SceneNode* node);

	~MeshComponent();

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	MeshComponent& setMeshFilename(CString fname);

	Bool isEnabled() const
	{
		return m_resource.isCreated();
	}

	const MeshResource& getMeshResource() const
	{
		return *m_resource;
	}

private:
	MeshResourcePtr m_resource;

	Bool m_resourceDirty = true;
};
/// @}

} // end namespace anki
