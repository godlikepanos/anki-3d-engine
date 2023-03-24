// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/DecalComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Shaders/Include/ClusteredShadingTypes.h>

namespace anki {

DecalComponent::DecalComponent(SceneNode* node)
	: SceneComponent(node, getStaticClassId())
	, m_node(node)
	, m_spatial(this)
{
	m_gpuSceneIndex =
		node->getSceneGraph().getAllGpuSceneContiguousArrays().allocate(GpuSceneContiguousArrayType::kDecals);
}

DecalComponent::~DecalComponent()
{
	m_spatial.removeFromOctree(m_node->getSceneGraph().getOctree());
}

void DecalComponent::onDestroy(SceneNode& node)
{
	node.getSceneGraph().getAllGpuSceneContiguousArrays().deferredFree(GpuSceneContiguousArrayType::kDecals,
																	   m_gpuSceneIndex);
}

void DecalComponent::setLayer(CString fname, F32 blendFactor, LayerType type)
{
	Layer& l = m_layers[type];

	ImageResourcePtr rsrc;

	const Error err = getExternalSubsystems(*m_node).m_resourceManager->loadResource(fname, rsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load image");
		return;
	}

	m_dirty = true;

	l.m_image = std::move(rsrc);
	l.m_bindlessTextureIndex = l.m_image->getTextureView()->getOrCreateBindlessTextureIndex();
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

		const Mat4 projMat = Mat4::calculateOrthographicProjectionMatrix(halfBoxSize.x(), -halfBoxSize.x(),
																		 halfBoxSize.y(), -halfBoxSize.y(),
																		 kClusterObjectFrustumNearPlane, m_boxSize.z());

		const Mat4 biasMat4(0.5f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
							1.0f);

		m_biasProjViewMat = biasMat4 * projMat * viewMat;

		// Update the spatial
		const Vec4 center(0.0f, 0.0f, -halfBoxSize.z(), 0.0f);
		const Vec4 extend(halfBoxSize.x(), halfBoxSize.y(), halfBoxSize.z(), 0.0f);
		const Obb obbL(center, Mat3x4::getIdentity(), extend);
		m_obb = obbL.getTransformed(info.m_node->getWorldTransform());
		m_spatial.setBoundingShape(m_obb);

		// Upload to the GPU scene
		GpuSceneDecal gpuDecal;
		gpuDecal.m_diffuseTexture = m_layers[LayerType::kDiffuse].m_bindlessTextureIndex;
		gpuDecal.m_roughnessMetalnessTexture = m_layers[LayerType::kRoughnessMetalness].m_bindlessTextureIndex;
		gpuDecal.m_diffuseBlendFactor = m_layers[LayerType::kDiffuse].m_blendFactor;
		gpuDecal.m_roughnessMetalnessFactor = m_layers[LayerType::kRoughnessMetalness].m_blendFactor;
		gpuDecal.m_textureMatrix = m_biasProjViewMat;

		const Mat4 trf(m_obb.getCenter().xyz1(), m_obb.getRotation().getRotationPart(), 1.0f);
		gpuDecal.m_invertedTransform = trf.getInverse();

		gpuDecal.m_obbExtend = m_obb.getExtend().xyz();

		const PtrSize offset = m_gpuSceneIndex * sizeof(GpuSceneDecal)
							   + info.m_node->getSceneGraph().getAllGpuSceneContiguousArrays().getArrayBase(
								   GpuSceneContiguousArrayType::kDecals);
		GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, offset, sizeof(gpuDecal), &gpuDecal);
	}

	const Bool spatialUpdated = m_spatial.update(info.m_node->getSceneGraph().getOctree());
	updated = updated || spatialUpdated;

	return Error::kNone;
}

} // end namespace anki
