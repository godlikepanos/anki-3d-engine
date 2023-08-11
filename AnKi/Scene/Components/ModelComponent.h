// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/RenderStateBucket.h>
#include <AnKi/Scene/Spatial.h>
#include <AnKi/Resource/Forward.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Renderer/RenderQueue.h>

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

	void setupRayTracingInstanceQueueElements(U32 lod, RenderingTechnique technique, WeakArray<RayTracingInstanceQueueElement>& outRenderables) const;

private:
	class PatchInfo
	{
	public:
		U32 m_gpuSceneUniformsOffset = kMaxU32;

		GpuSceneArrays::MeshLod::Allocation m_gpuSceneMeshLods;
		GpuSceneArrays::Renderable::Allocation m_gpuSceneRenderable;
		GpuSceneArrays::RenderableAabbGBuffer::Allocation m_gpuSceneRenderableAabbGBuffer;
		GpuSceneArrays::RenderableAabbDepth::Allocation m_gpuSceneRenderableAabbDepth;
		GpuSceneArrays::RenderableAabbForward::Allocation m_gpuSceneRenderableAabbForward;

		Array<RenderStateBucketIndex, U32(RenderingTechnique::kCount)> m_renderStateBucketIndices;
		RenderingTechniqueBit m_techniques;
	};

	SceneNode* m_node = nullptr;
	SkinComponent* m_skinComponent = nullptr;

	ModelResourcePtr m_model;

	GpuSceneBufferAllocation m_gpuSceneUniforms;
	GpuSceneArrays::Transform::Allocation m_gpuSceneTransforms;
	SceneDynamicArray<PatchInfo> m_patchInfos;

	Bool m_resourceChanged : 1 = true;
	Bool m_castsShadow : 1 = false;
	Bool m_movedLastFrame : 1 = true;
	Bool m_firstTimeUpdate : 1 = true; ///< Extra flag in case the component is added in a node that hasn't been moved.

	RenderingTechniqueBit m_presentRenderingTechniques = RenderingTechniqueBit::kNone;

	void freeGpuScene();

	Error update(SceneComponentUpdateInfo& info, Bool& updated) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;

	Aabb computeAabbWorldSpace(const Transform& worldTransform) const;
};
/// @}

} // end namespace anki
