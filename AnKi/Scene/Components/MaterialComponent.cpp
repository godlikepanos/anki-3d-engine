// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MaterialComponent.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Core/App.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>

namespace anki {

MaterialComponent::MaterialComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_gpuSceneRenderable.allocate();
}

MaterialComponent::~MaterialComponent()
{
	m_gpuSceneRenderable.free();
}

MaterialComponent& MaterialComponent::setMaterialFilename(CString fname)
{
	MaterialResourcePtr newRsrc;
	if(ResourceManager::getSingleton().loadResource(fname, newRsrc))
	{
		ANKI_SCENE_LOGE("Failed to load resource: %s", fname.cstr());
	}
	else
	{
		m_resourceDirty = !m_resource || (m_resource->getUuid() != newRsrc->getUuid());
		m_resource = std::move(newRsrc);
		m_castsShadow = m_resource->castsShadow();
	}

	return *this;
}

CString MaterialComponent::getMaterialFilename() const
{
	if(m_resource)
	{
		return m_resource->getFilename();
	}
	else
	{
		return "*Error*";
	}
}

MaterialComponent& MaterialComponent::setSubmeshIndex(U32 submeshIdx)
{
	if(m_submeshIdx != submeshIdx)
	{
		m_submeshIdx = submeshIdx;
		m_submeshIdxDirty = true;
	}

	return *this;
}

void MaterialComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	ANKI_ASSERT(other);

	if(other->getType() == SceneComponentType::kMesh)
	{
		Bool dirty;
		bookkeepComponent(m_meshComponents, other, added, dirty);
		m_meshComponentDirty = dirty;
	}

	if(other->getType() == SceneComponentType::kSkin)
	{
		Bool dirty;
		bookkeepComponent(m_skinComponents, other, added, dirty);
		m_skinDirty = dirty;
	}
}

Aabb MaterialComponent::computeAabb(U32 submeshIndex, const SceneNode& node) const
{
	U32 firstIndex, indexCount, firstMeshlet, meshletCount;
	Aabb aabbLocal;
	m_meshComponents[0]->getMeshResource().getSubMeshInfo(0, submeshIndex, firstIndex, indexCount, firstMeshlet, meshletCount, aabbLocal);

	if(m_skinComponents.getSize() && m_skinComponents[0]->isValid())
	{
		aabbLocal = m_skinComponents[0]->getBoneBoundingVolumeLocalSpace().getCompoundShape(aabbLocal);
	}

	const Aabb aabbWorld = aabbLocal.getTransformed(node.getWorldTransform());
	return aabbWorld;
}

Bool MaterialComponent::isValid() const
{
	Bool valid = !!m_resource && m_resource->isLoaded() && m_meshComponents.getSize() && m_meshComponents[0]->isValid();
	if(m_skinComponents.getSize())
	{
		valid = valid && m_skinComponents[0]->isValid();
	}
	return valid;
}

void MaterialComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!isValid()) [[unlikely]]
	{
		m_gpuSceneRenderableAabbGBuffer.free();
		m_gpuSceneRenderableAabbDepth.free();
		m_gpuSceneRenderableAabbForward.free();
		m_gpuSceneRenderableAabbRt.free();

		for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
		{
			RenderStateBucketContainer::getSingleton().removeUser(m_renderStateBucketIndices[t]);
		}

		return;
	}

	// From now on the component is considered valid

	const Bool mtlResourceChanged = m_resourceDirty;
	const Bool meshChanged = m_meshComponentDirty || m_meshComponents[0]->gpuSceneReallocationsThisFrame();
	const Bool moved = info.m_node->movedThisFrame() || m_firstTimeUpdate;
	const Bool movedLastFrame = m_movedLastFrame || m_firstTimeUpdate;
	const Bool hasSkin = m_skinComponents.getSize();
	const Bool skinChanged = m_skinDirty || (hasSkin && m_skinComponents[0]->gpuSceneReallocationsThisFrame());
	const Bool submeshChanged = m_submeshIdxDirty;

	updated = mtlResourceChanged || meshChanged || moved || skinChanged || submeshChanged;

	m_resourceDirty = false;
	m_firstTimeUpdate = false;
	m_meshComponentDirty = false;
	m_movedLastFrame = moved;
	m_skinDirty = false;
	m_submeshIdxDirty = false;

	const MaterialResource& mtl = *m_resource;
	const MeshResource& mesh = m_meshComponents[0]->getMeshResource();
	const U32 submeshIdx = min(mesh.getSubMeshCount() - 1, m_submeshIdx);

	// Extract the diffuse color
	Vec3 averageDiffuse(0.0f);
	if(mtlResourceChanged)
	{
		const MaterialVariable* diffuseRelatedMtlVar = nullptr;

		for(const MaterialVariable& mtlVar : mtl.getVariables())
		{
			SceneString name = mtlVar.getName();
			name.toLower();

			if(name.find("diffuse") != String::kNpos || name.find("albedo") != String::kNpos)
			{
				if(diffuseRelatedMtlVar)
				{
					if(name.find("tex") != String::kNpos)
					{
						diffuseRelatedMtlVar = &mtlVar;
					}
				}
				else
				{
					diffuseRelatedMtlVar = &mtlVar;
				}
			}
		}

		if(diffuseRelatedMtlVar)
		{
			if(diffuseRelatedMtlVar->getDataType() >= ShaderVariableDataType::kTextureFirst
			   && diffuseRelatedMtlVar->getDataType() <= ShaderVariableDataType::kTextureLast)
			{
				averageDiffuse = diffuseRelatedMtlVar->getValue<ImageResourcePtr>()->getAverageColor().xyz();
			}
			else if(diffuseRelatedMtlVar->getDataType() == ShaderVariableDataType::kVec3)
			{
				averageDiffuse = diffuseRelatedMtlVar->getValue<Vec3>();
			}
			else if(diffuseRelatedMtlVar->getDataType() == ShaderVariableDataType::kU32 && diffuseRelatedMtlVar->tryGetImageResource())
			{
				// Bindless texture
				averageDiffuse = diffuseRelatedMtlVar->tryGetImageResource()->getAverageColor().xyz();
			}
			else
			{
				ANKI_SCENE_LOGW("Couldn't extract a diffuse value for material: %s", mtl.getFilename().cstr());
			}
		}
	}

	// Update the constants
	Bool constantsReallocated = false;
	if(mtlResourceChanged) [[unlikely]]
	{
		ConstWeakArray<U8> preallocatedConsts = mtl.getPrefilledLocalConstants();

		if(!m_gpuSceneConstants || m_gpuSceneConstants.getAllocatedSize() != preallocatedConsts.getSizeInBytes())
		{
			GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneConstants);
			m_gpuSceneConstants = GpuSceneBuffer::getSingleton().allocate(preallocatedConsts.getSizeInBytes(), 4);
			constantsReallocated = true;
		}

		GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, m_gpuSceneConstants.getOffset(), m_gpuSceneConstants.getAllocatedSize(),
													 preallocatedConsts.getBegin());
	}

	// Update renderable
	if(mtlResourceChanged || constantsReallocated || skinChanged || meshChanged || submeshChanged) [[unlikely]]
	{
		const MoveComponent& movec = info.m_node->getFirstComponentOfType<MoveComponent>();

		GpuSceneRenderable gpuRenderable = {};
		gpuRenderable.m_worldTransformsIndex = movec.getGpuSceneTransforms().getIndex() * 2;
		gpuRenderable.m_constantsOffset = m_gpuSceneConstants.getOffset();
		gpuRenderable.m_meshLodsIndex = m_meshComponents[0]->getGpuSceneMeshLods(submeshIdx).getIndex() * kMaxLodCount;
		gpuRenderable.m_boneTransformsOffset = (hasSkin) ? m_skinComponents[0]->getBoneTransformsGpuSceneOffset() : 0;
		gpuRenderable.m_particleEmitterIndex = kMaxU32;
		if(!!(mtl.getRenderingTechniques() & RenderingTechniqueBit::kRtShadow))
		{
			const RenderingKey key(RenderingTechnique::kRtShadow, 0, false, false, false);
			const MaterialVariant& variant = mtl.getOrCreateVariant(key);
			gpuRenderable.m_rtShadowsShaderHandleIndex = variant.getRtShaderGroupHandleIndex();
		}
		if(!!(mtl.getRenderingTechniques() & RenderingTechniqueBit::kRtMaterialFetch))
		{
			const RenderingKey key(RenderingTechnique::kRtMaterialFetch, 0, false, false, false);
			const MaterialVariant& variant = mtl.getOrCreateVariant(key);
			gpuRenderable.m_rtMaterialFetchShaderHandleIndex = variant.getRtShaderGroupHandleIndex();
		}
		gpuRenderable.m_uuid = SceneGraph::getSingleton().getNewUuid();

		const UVec3 u3(averageDiffuse.xyz().clamp(0.0f, 1.0f) * 255.0f);
		gpuRenderable.m_diffuseColor = ((u3.x() << 16u) | (u3.y() << 8u) | u3.z()) & 0xFFFFFFF;

		m_gpuSceneRenderable.uploadToGpuScene(gpuRenderable);
	}

	// Scene bounds update
	const Bool aabbUpdated = moved || meshChanged || submeshChanged || hasSkin;
	if(aabbUpdated || info.m_forceUpdateSceneBounds) [[unlikely]]
	{
		const Aabb aabbWorld = computeAabb(submeshIdx, *info.m_node);
		info.updateSceneBounds(aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz());
	}

	// Update the buckets
	const Bool bucketsNeedUpdate = mtlResourceChanged || submeshChanged || moved != movedLastFrame || skinChanged;
	if(bucketsNeedUpdate) [[unlikely]]
	{
		for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
		{
			RenderStateBucketContainer::getSingleton().removeUser(m_renderStateBucketIndices[t]);

			if(!(RenderingTechniqueBit(1 << t) & mtl.getRenderingTechniques()))
			{
				continue;
			}

			// Fill the state
			RenderingKey key;
			key.setLod(0); // Materials don't care
			key.setRenderingTechnique(t);
			key.setSkinned(hasSkin);
			key.setVelocity(moved);
			key.setMeshletRendering(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders || g_cvarCoreMeshletRendering);

			const MaterialVariant& mvariant = mtl.getOrCreateVariant(key);

			RenderStateInfo state;
			state.m_primitiveTopology = PrimitiveTopology::kTriangles;
			state.m_program = mvariant.getShaderProgram();

			U32 firstIndex, indexCount, firstMeshlet, meshletCount;
			Aabb aabb;
			mesh.getSubMeshInfo(0, submeshIdx, firstIndex, indexCount, firstMeshlet, meshletCount, aabb);
			const Bool wantsMesletCount = key.getMeshletRendering() && !(RenderingTechniqueBit(1 << t) & RenderingTechniqueBit::kAllRt);

			m_renderStateBucketIndices[t] = RenderStateBucketContainer::getSingleton().addUser(state, t, (wantsMesletCount) ? meshletCount : 0);
		}
	}

	// Upload the AABBs to the GPU scene
	const Bool gpuSceneAabbsNeedUpdate = aabbUpdated || bucketsNeedUpdate;
	if(gpuSceneAabbsNeedUpdate) [[unlikely]]
	{
		const Aabb aabbWorld = computeAabb(submeshIdx, *info.m_node);

		// Raster
		for(RenderingTechnique t : EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(RenderingTechniqueBit::kAllRaster))
		{
			const RenderingTechniqueBit bit = RenderingTechniqueBit(1 << t);
			if(!(mtl.getRenderingTechniques() & bit))
			{
				switch(t)
				{
				case RenderingTechnique::kGBuffer:
					m_gpuSceneRenderableAabbGBuffer.free();
					break;
				case RenderingTechnique::kDepth:
					m_gpuSceneRenderableAabbDepth.free();
					break;
				case RenderingTechnique::kForward:
					m_gpuSceneRenderableAabbForward.free();
					break;
				default:
					ANKI_ASSERT(0);
				}
			}
			else
			{
				const GpuSceneRenderableBoundingVolume gpuVolume = initGpuSceneRenderableBoundingVolume(
					aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz(), m_gpuSceneRenderable.getIndex(), m_renderStateBucketIndices[t].get());

				switch(t)
				{
				case RenderingTechnique::kGBuffer:
					if(!m_gpuSceneRenderableAabbGBuffer.isValid())
					{
						m_gpuSceneRenderableAabbGBuffer.allocate();
					}
					m_gpuSceneRenderableAabbGBuffer.uploadToGpuScene(gpuVolume);
					break;
				case RenderingTechnique::kDepth:
					if(!m_gpuSceneRenderableAabbDepth.isValid())
					{
						m_gpuSceneRenderableAabbDepth.allocate();
					}
					m_gpuSceneRenderableAabbDepth.uploadToGpuScene(gpuVolume);
					break;
				case RenderingTechnique::kForward:
					if(!m_gpuSceneRenderableAabbForward.isValid())
					{
						m_gpuSceneRenderableAabbForward.allocate();
					}
					m_gpuSceneRenderableAabbForward.uploadToGpuScene(gpuVolume);
					break;
				default:
					ANKI_ASSERT(0);
				}
			}
		}

		// RT
		if(!!(mtl.getRenderingTechniques() & RenderingTechniqueBit::kAllRt))
		{
			if(!m_gpuSceneRenderableAabbRt.isValid())
			{
				m_gpuSceneRenderableAabbRt.allocate();
			}

			const U32 bucketIdx = 0;
			const GpuSceneRenderableBoundingVolume gpuVolume =
				initGpuSceneRenderableBoundingVolume(aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz(), m_gpuSceneRenderable.getIndex(), bucketIdx);

			m_gpuSceneRenderableAabbRt.uploadToGpuScene(gpuVolume);
		}
		else
		{
			m_gpuSceneRenderableAabbRt.free();
		}
	}
}

} // end namespace anki
