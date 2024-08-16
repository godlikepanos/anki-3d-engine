// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/GpuSceneArray.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Collision/Aabb.h>

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

	void loadModelResource(CString filename);

	const ModelResourcePtr& getModelResource() const
	{
		return m_model;
	}

	Bool isEnabled() const
	{
		return m_model.isCreated();
	}

	Bool getCastsShadow() const
	{
		return m_castsShadow;
	}

private:
	class PatchInfo
	{
	public:
		U32 m_gpuSceneConstantsOffset = kMaxU32;

		GpuSceneArrays::MeshLod::Allocation m_gpuSceneMeshLods;
		GpuSceneArrays::Renderable::Allocation m_gpuSceneRenderable;
		GpuSceneArrays::RenderableBoundingVolumeGBuffer::Allocation m_gpuSceneRenderableAabbGBuffer;
		GpuSceneArrays::RenderableBoundingVolumeDepth::Allocation m_gpuSceneRenderableAabbDepth;
		GpuSceneArrays::RenderableBoundingVolumeForward::Allocation m_gpuSceneRenderableAabbForward;
		GpuSceneArrays::RenderableBoundingVolumeRt::Allocation m_gpuSceneRenderableAabbRt;

		Array<RenderStateBucketIndex, U32(RenderingTechnique::kCount)> m_renderStateBucketIndices;
		RenderingTechniqueBit m_techniques;
	};

	SkinComponent* m_skinComponent = nullptr;

	ModelResourcePtr m_model;

	// GPU scene part 1
	GpuSceneBufferAllocation m_gpuSceneConstants;
	GpuSceneArrays::Transform::Allocation m_gpuSceneTransforms;

	// Other stuff
	Bool m_resourceChanged : 1 = true;
	Bool m_castsShadow : 1 = false;
	Bool m_movedLastFrame : 1 = true;
	Bool m_firstTimeUpdate : 1 = true; ///< Extra flag in case the component is added in a node that hasn't been moved.

	RenderingTechniqueBit m_presentRenderingTechniques = RenderingTechniqueBit::kNone;

	// GPU scene part 2
	SceneDynamicArray<PatchInfo> m_patchInfos;

	void freeGpuScene();

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;

	Aabb computeAabbWorldSpace(const Transform& worldTransform) const;
};
/// @}

} // end namespace anki
