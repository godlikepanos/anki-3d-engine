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

// Connects the material resource with geometry based components (mesh or particle emitter)
class MaterialComponent final : public SceneComponent
{
	ANKI_SCENE_COMPONENT(MaterialComponent)

public:
	MaterialComponent(const SceneComponentInitInfo& init);

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

	Bool isValid() const;

private:
	GpuSceneArrays::Renderable::Allocation m_gpuSceneRenderable;
	GpuSceneArrays::RenderableBoundingVolumeGBuffer::Allocation m_gpuSceneRenderableAabbGBuffer;
	GpuSceneArrays::RenderableBoundingVolumeDepth::Allocation m_gpuSceneRenderableAabbDepth;
	GpuSceneArrays::RenderableBoundingVolumeForward::Allocation m_gpuSceneRenderableAabbForward;
	GpuSceneArrays::RenderableBoundingVolumeRt::Allocation m_gpuSceneRenderableAabbRt;
	GpuSceneBufferAllocation m_gpuSceneConstants;

	Array<RenderStateBucketIndex, U32(RenderingTechnique::kCount)> m_renderStateBucketIndices;

	MaterialResourcePtr m_resource;

	SkinComponent* m_skinComponent = nullptr;
	MeshComponent* m_meshComponent = nullptr;
	ParticleEmitter2Component* m_emitterComponent = nullptr;

	U32 m_submeshIdx = 0;

	Bool m_anyDirty : 1 = true; // A compound flag because it's too difficult to track everything
	Bool m_movedLastFrame : 1 = true;

	static inline Atomic<U32> m_renderableUuid = {1};

	void update(SceneComponentUpdateInfo& info, Bool& updated) override;

	Error serialize(SceneSerializer& serializer) override;

	void onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added) override;

	Aabb computeAabb(const SceneNode& node) const;
};

} // end namespace anki
