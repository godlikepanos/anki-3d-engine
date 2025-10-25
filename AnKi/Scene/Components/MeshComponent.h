// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Scene/GpuSceneArray.h>

namespace anki {

// Holds geometry information.
class MeshComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(MeshComponent)

public:
	MeshComponent(SceneNode* node);

	~MeshComponent();

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	MeshComponent& setMeshFilename(CString fname);

	Bool hasMeshResource() const
	{
		return !!m_resource;
	}

	CString getMeshFilename() const;

	Bool isValid() const;

	const MeshResource& getMeshResource() const
	{
		ANKI_ASSERT(isValid());
		return *m_resource;
	}

	ANKI_INTERNAL const GpuSceneArrays::MeshLod::Allocation& getGpuSceneMeshLods(U32 submeshIdx) const
	{
		ANKI_ASSERT(isValid());
		return m_gpuSceneMeshLods[submeshIdx];
	}

	ANKI_INTERNAL Bool gpuSceneReallocationsThisFrame() const
	{
		ANKI_ASSERT(isValid());
		return m_gpuSceneMeshLodsReallocatedThisFrame;
	}

private:
	MeshResourcePtr m_resource;

	SceneDynamicArray<GpuSceneArrays::MeshLod::Allocation> m_gpuSceneMeshLods;

	Bool m_resourceDirty = true;
	Bool m_gpuSceneMeshLodsReallocatedThisFrame = false;
};

} // end namespace anki
