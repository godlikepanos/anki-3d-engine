// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>

namespace anki {

FogDensityComponent::FogDensityComponent(SceneNode* node, U32 uuid)
	: SceneComponent(node, kClassType, uuid)
{
	m_gpuSceneVolume.allocate();
}

FogDensityComponent ::~FogDensityComponent()
{
}

void FogDensityComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	ANKI_ASSERT(m_type < FogDensityComponentShape::kCount);

	updated = m_dirty || info.m_node->movedThisFrame();

	if(updated)
	{
		m_dirty = false;

		const Transform& trf = info.m_node->getWorldTransform();

		// Upload to the GPU scene
		GpuSceneFogDensityVolume gpuVolume;
		if(m_type == FogDensityComponentShape::kBox)
		{
			gpuVolume.m_aabbMinOrSphereCenter = Vec3(-1.0f) * trf.getScale().xyz + trf.getOrigin().xyz;
			gpuVolume.m_aabbMaxOrSphereRadius = Vec3(+1.0f) * trf.getScale().xyz + trf.getOrigin().xyz;
		}
		else
		{
			gpuVolume.m_aabbMaxOrSphereRadius = Vec3(1.0f * max(max(trf.getScale().x, trf.getScale().y), trf.getScale().z));
			gpuVolume.m_aabbMinOrSphereCenter = trf.getOrigin().xyz;
		}
		gpuVolume.m_isBox = m_type == FogDensityComponentShape::kBox;
		gpuVolume.m_density = m_density;
		gpuVolume.m_sceneNodeUuid = info.m_node->getUuid();

		m_gpuSceneVolume.uploadToGpuScene(gpuVolume);
	}
}

Error FogDensityComponent::serialize(SceneSerializer& serializer)
{
	U32 type = U32(m_type);
	ANKI_SERIALIZE(type, 1);
	m_type = FogDensityComponentShape(type);

	ANKI_SERIALIZE(m_density, 1);

	return Error::kNone;
}

} // end namespace anki
