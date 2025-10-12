// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Resource/Forward.h>

namespace anki {

// Forward
class Aabb;

/// @addtogroup scene
/// @{

/// Holds geometry information.
class MaterialComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(MaterialComponent)

public:
	MaterialComponent(SceneNode* node);

	~MaterialComponent();

	MaterialComponent& setMaterialFilename(CString fname);

	CString getMaterialFilename() const;

	Bool hasMaterialResource() const
	{
		return !!m_resource;
	}

	MaterialComponent& setSubmeshIndex(U32 submeshIdx);

	U32 getSubmeshIndex() const
	{
		return m_submeshIdx;
	}

private:
	GpuSceneArrays::MeshLod::Allocation m_gpuSceneMeshLods;
	GpuSceneArrays::Renderable::Allocation m_gpuSceneRenderable;
	GpuSceneArrays::RenderableBoundingVolumeGBuffer::Allocation m_gpuSceneRenderableAabbGBuffer;
	GpuSceneArrays::RenderableBoundingVolumeDepth::Allocation m_gpuSceneRenderableAabbDepth;
	GpuSceneArrays::RenderableBoundingVolumeForward::Allocation m_gpuSceneRenderableAabbForward;
	GpuSceneArrays::RenderableBoundingVolumeRt::Allocation m_gpuSceneRenderableAabbRt;
	GpuSceneArrays::Transform::Allocation m_gpuSceneTransforms;
	GpuSceneBufferAllocation m_gpuSceneConstants;

	Array<RenderStateBucketIndex, U32(RenderingTechnique::kCount)> m_renderStateBucketIndices;

	MaterialResourcePtr m_resource;

	SkinComponent* m_skinComponent = nullptr;
	MeshComponent* m_meshComponent = nullptr;

	U32 m_submeshIdx = 0;

	Bool m_resourceDirty : 1 = true;
	Bool m_skinDirty : 1 = true;
	Bool m_meshComponentDirty : 1 = true;
	Bool m_firstTimeUpdate : 1 = true; ///< Extra flag in case the component is added in a node that hasn't been moved.
	Bool m_movedLastFrame : 1 = true;
	Bool m_submeshIdxDirty : 1 = true;
	Bool m_castsShadow : 1 = false;

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;

	Aabb computeAabb(U32 submeshIndex, const SceneNode& node) const;
};
/// @}

} // end namespace anki
