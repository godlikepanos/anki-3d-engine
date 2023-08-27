// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>
#include <AnKi/Core/GpuMemory/GpuSceneBuffer.h>

namespace anki {

DecalComponent::DecalComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_gpuSceneDecal.allocate();
}

DecalComponent::~DecalComponent()
{
}

void DecalComponent::setLayer(CString fname, F32 blendFactor, LayerType type)
{
	Layer& l = m_layers[type];

	ImageResourcePtr rsrc;

	const Error err = ResourceManager::getSingleton().loadResource(fname, rsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load image");
		return;
	}

	m_dirty = true;

	l.m_image = std::move(rsrc);
	l.m_bindlessTextureIndex = l.m_image->getTextureView().getOrCreateBindlessTextureIndex();
	l.m_blendFactor = blendFactor;
}

Error DecalComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_dirty || info.m_node->movedThisFrame();

	if(updated)
	{
		m_dirty = false;

		const Vec3 halfBoxSize = m_boxSize / 2.0f;

		// Calculate the texture matrix
		const Mat4 worldTransform(info.m_node->getWorldTransform());

		const Mat4 viewMat = worldTransform.getInverse();

		const Mat4 projMat = Mat4::calculateOrthographicProjectionMatrix(halfBoxSize.x(), -halfBoxSize.x(), halfBoxSize.y(), -halfBoxSize.y(),
																		 kClusterObjectFrustumNearPlane, m_boxSize.z());

		const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

		m_biasProjViewMat = biasMat4 * projMat * viewMat;

		// Update the spatial
		const Vec4 center(0.0f, 0.0f, -halfBoxSize.z(), 0.0f);
		const Vec4 extend(halfBoxSize.x(), halfBoxSize.y(), halfBoxSize.z(), 0.0f);
		const Obb obbL(center, Mat3x4::getIdentity(), extend);
		const Obb obbW = obbL.getTransformed(info.m_node->getWorldTransform());

		// Upload to the GPU scene
		GpuSceneDecal gpuDecal;
		gpuDecal.m_diffuseTexture = m_layers[LayerType::kDiffuse].m_bindlessTextureIndex;
		gpuDecal.m_roughnessMetalnessTexture = m_layers[LayerType::kRoughnessMetalness].m_bindlessTextureIndex;
		gpuDecal.m_diffuseBlendFactor = m_layers[LayerType::kDiffuse].m_blendFactor;
		gpuDecal.m_roughnessMetalnessFactor = m_layers[LayerType::kRoughnessMetalness].m_blendFactor;
		gpuDecal.m_textureMatrix = m_biasProjViewMat;
		gpuDecal.m_sphereCenter = obbW.getCenter().xyz();
		gpuDecal.m_sphereRadius = obbW.getExtend().getLength();

		m_gpuSceneDecal.uploadToGpuScene(gpuDecal);
	}

	return Error::kNone;
}

} // end namespace anki
