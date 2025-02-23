// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
	loadDiffuseImageResource("EngineAssets/DefaultDecal.png", 0.9f);
}

DecalComponent::~DecalComponent()
{
}

void DecalComponent::setLayer(CString fname, F32 blendFactor, LayerType type)
{
	Layer& l = m_layers[type];

	ImageResourcePtr rsrc;
	if(ResourceManager::getSingleton().loadResource(fname, rsrc))
	{
		ANKI_SCENE_LOGE("Failed to load image");
		return;
	}

	m_dirty = true;

	l.m_image = std::move(rsrc);
	l.m_bindlessTextureIndex = l.m_image->getTexture().getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
	l.m_blendFactor = clamp(blendFactor, 0.0f, 1.0f);
}

Error DecalComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_dirty || info.m_node->movedThisFrame();

	if(!updated) [[likely]]
	{
		return Error::kNone;
	}

	m_dirty = false;

	const Vec3 halfBoxSize = info.m_node->getWorldTransform().getScale().xyz();

	// Calculate the texture matrix
	Transform trf = info.m_node->getWorldTransform();
	trf.setScale(Vec3(1.0f));
	const Mat4 viewMat = Mat4(trf).invert();

	const Mat4 projMat = Mat4::calculateOrthographicProjectionMatrix(halfBoxSize.x(), -halfBoxSize.x(), halfBoxSize.y(), -halfBoxSize.y(),
																	 -halfBoxSize.z(), halfBoxSize.z());

	const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, -0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

	const Mat4 biasedProjViewMat = biasMat4 * projMat * viewMat;

	// Upload to the GPU scene
	GpuSceneDecal gpuDecal;
	gpuDecal.m_diffuseTexture = m_layers[LayerType::kDiffuse].m_bindlessTextureIndex;
	gpuDecal.m_roughnessMetalnessTexture = m_layers[LayerType::kRoughnessMetalness].m_bindlessTextureIndex;
	gpuDecal.m_diffuseBlendFactor = m_layers[LayerType::kDiffuse].m_blendFactor;
	gpuDecal.m_roughnessMetalnessFactor = m_layers[LayerType::kRoughnessMetalness].m_blendFactor;
	gpuDecal.m_textureMatrix = biasedProjViewMat;
	gpuDecal.m_sphereCenter = info.m_node->getWorldTransform().getOrigin().xyz();
	gpuDecal.m_sphereRadius = halfBoxSize.length();

	m_gpuSceneDecal.uploadToGpuScene(gpuDecal);

	return Error::kNone;
}

} // end namespace anki
