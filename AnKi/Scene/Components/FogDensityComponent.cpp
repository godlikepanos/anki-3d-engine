// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/FogDensityComponent.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>

namespace anki {

FogDensityComponent::FogDensityComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_gpuSceneVolume.allocate();
}

FogDensityComponent ::~FogDensityComponent()
{
}

Error FogDensityComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_dirty || info.m_node->movedThisFrame();

	if(updated)
	{
		m_dirty = false;

		m_worldPos = info.m_node->getWorldTransform().getOrigin().xyz();

		// Upload to the GPU scene
		GpuSceneFogDensityVolume gpuVolume;
		if(m_isBox)
		{
			gpuVolume.m_aabbMinOrSphereCenter = m_aabbMin.xyz();
			gpuVolume.m_aabbMaxOrSphereRadius = m_aabbMax.xyz();
		}
		else
		{
			gpuVolume.m_aabbMaxOrSphereRadius = Vec3(m_sphereRadius);
			gpuVolume.m_aabbMinOrSphereCenter = m_worldPos.xyz();
		}
		gpuVolume.m_isBox = m_isBox;
		gpuVolume.m_density = m_density;

		m_gpuSceneVolume.uploadToGpuScene(gpuVolume);
	}

	return Error::kNone;
}

} // end namespace anki
