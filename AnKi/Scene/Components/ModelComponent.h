// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Holds geometry and material information.
class ModelComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(ModelComponent)

public:
	ModelComponent(SceneNode* node);

	~ModelComponent();

	Error loadModelResource(CString filename);

	const ModelResourcePtr& getModelResource() const
	{
		return m_model;
	}

	ConstWeakArray<U64> getRenderMergeKeys() const
	{
		return m_modelPatchMergeKeys;
	}

	Bool isEnabled() const
	{
		return m_model.isCreated();
	}

	U32 getMeshViewsGpuSceneOffset() const
	{
		ANKI_ASSERT((m_gpuSceneMeshGpuViews.m_offset % 4) == 0);
		return U32(m_gpuSceneMeshGpuViews.m_offset);
	}

	U32 getUniformsGpuSceneOffset(U32 meshPatchIdx) const
	{
		return m_gpuSceneUniformsOffsetPerPatch[meshPatchIdx];
	}

private:
	SceneNode* m_node = nullptr;
	ModelResourcePtr m_model;

	DynamicArray<U64> m_modelPatchMergeKeys;
	Bool m_dirty = true;

	SegregatedListsGpuMemoryPoolToken m_gpuSceneMeshGpuViews;
	SegregatedListsGpuMemoryPoolToken m_gpuSceneUniforms;
	DynamicArray<U32> m_gpuSceneUniformsOffsetPerPatch;

	Error update(SceneComponentUpdateInfo& info, Bool& updated);
};
/// @}

} // end namespace anki
