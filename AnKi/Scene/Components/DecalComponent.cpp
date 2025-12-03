// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>
#include <AnKi/GpuMemory/GpuSceneBuffer.h>

namespace anki {

DecalComponent::DecalComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_gpuSceneDecal.allocate();
	setDiffuseImageFilename("EngineAssets/DefaultDecal.png");
	setDiffuseBlendFactor(0.9f);

	m_defaultDecalImage = m_layers[LayerType::kDiffuse].m_image;
}

DecalComponent::~DecalComponent()
{
}

void DecalComponent::setImage(LayerType type, CString fname)
{
	ImageResourcePtr rsrc;
	if(ANKI_EXPECT(type < LayerType::kCount && !ResourceManager::getSingleton().loadResource(fname, rsrc)))
	{
		Layer& l = m_layers[type];
		if(!l.m_image || rsrc->getUuid() != l.m_image->getUuid())
		{
			m_dirty = true;

			l.m_image = std::move(rsrc);
			l.m_bindlessTextureIndex = l.m_image->getTexture().getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
		}
	}
}

void DecalComponent::setBlendFactor(LayerType type, F32 blendFactor)
{
	if(ANKI_EXPECT(type < LayerType::kCount))
	{
		Layer& l = m_layers[type];
		blendFactor = saturate(blendFactor);
		if(l.m_blendFactor != blendFactor)
		{
			l.m_blendFactor = blendFactor;
			m_dirty = true;
		}
	}
}

void DecalComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	updated = m_dirty || info.m_node->movedThisFrame();

	if(!updated) [[likely]]
	{
		return;
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
}

} // end namespace anki
