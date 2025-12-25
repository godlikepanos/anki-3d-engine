// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MaterialComponent.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Scene/Components/ParticleEmitter2Component.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Core/App.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>

namespace anki {

MaterialComponent::MaterialComponent(SceneNode* node, U32 uuid)
	: SceneComponent(node, kClassType, uuid)
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
		m_anyDirty = !m_resource || (m_resource->getUuid() != newRsrc->getUuid());
		m_resource = std::move(newRsrc);
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
		m_anyDirty = true;
	}

	return *this;
}

void MaterialComponent::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	ANKI_ASSERT(other);

	if(other->getType() == SceneComponentType::kMesh)
	{
		bookkeepComponent(m_meshComponent, other, added);
		m_anyDirty = true;
	}

	if(other->getType() == SceneComponentType::kSkin)
	{
		bookkeepComponent(m_skinComponent, other, added);
		m_anyDirty = true;
	}

	if(other->getType() == SceneComponentType::kParticleEmitter2)
	{
		bookkeepComponent(m_emitterComponent, other, added);
		m_anyDirty = true;
	}
}

Bool MaterialComponent::isValid() const
{
	Bool valid = !!m_resource && m_resource->isLoaded();

	if(m_meshComponent)
	{
		valid = valid && m_meshComponent->isValid();
	}

	if(m_emitterComponent)
	{
		valid = valid && m_emitterComponent->isValid();
	}

	valid = valid && (m_meshComponent || m_emitterComponent);

	if(m_skinComponent)
	{
		valid = valid && m_skinComponent->isValid();
	}
	return valid;
}

Aabb MaterialComponent::computeAabb(const SceneNode& node) const
{
	const Bool prioritizeEmitter = m_emitterComponent != nullptr;
	Aabb aabbWorld;
	if(prioritizeEmitter)
	{
		aabbWorld = Aabb(m_emitterComponent->getBoundingVolume()[0], m_emitterComponent->getBoundingVolume()[1]);
	}
	else
	{
		Aabb aabbLocal;
		U32 firstIndex, indexCount, firstMeshlet, meshletCount;
		m_meshComponent->getMeshResource().getSubMeshInfo(0, m_submeshIdx, firstIndex, indexCount, firstMeshlet, meshletCount, aabbLocal);

		if(m_skinComponent && m_skinComponent->isValid())
		{
			aabbLocal = m_skinComponent->getBoneBoundingVolumeLocalSpace().getCompoundShape(aabbLocal);
		}

		aabbWorld = aabbLocal.getTransformed(node.getWorldTransform());
	}

	return aabbWorld;
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

	const Bool moved = info.m_node->movedThisFrame();
	const Bool movedLastFrame = m_movedLastFrame;
	m_movedLastFrame = moved;

	Bool dirty = m_anyDirty || moved != movedLastFrame;

	const Bool prioritizeEmitter = !!m_emitterComponent;
	const MaterialResource& mtl = *m_resource;

	if(m_skinComponent)
	{
		dirty = dirty || m_skinComponent->gpuSceneReallocationsThisFrame();
	}

	if(m_emitterComponent)
	{
		dirty = dirty || m_emitterComponent->gpuSceneReallocationsThisFrame();
	}

	if(m_meshComponent)
	{
		dirty = dirty || m_meshComponent->gpuSceneReallocationsThisFrame();
	}

	if(!dirty) [[likely]]
	{
		// Update Scene bounds
		if(info.m_forceUpdateSceneBounds || m_skinComponent) [[unlikely]]
		{
			const Aabb aabbWorld = computeAabb(*info.m_node);
			info.updateSceneBounds(aabbWorld.getMin().xyz, aabbWorld.getMax().xyz);
		}

		// Update the GPU scene AABBs
		if(prioritizeEmitter || m_skinComponent || moved)
		{
			const Aabb aabbWorld = computeAabb(*info.m_node);
			for(RenderingTechnique t : EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(mtl.getRenderingTechniques()))
			{
				const GpuSceneRenderableBoundingVolume gpuVolume = initGpuSceneRenderableBoundingVolume(
					aabbWorld.getMin().xyz, aabbWorld.getMax().xyz, m_gpuSceneRenderable.getIndex(), m_renderStateBucketIndices[t].get());

				switch(t)
				{
				case RenderingTechnique::kGBuffer:
					m_gpuSceneRenderableAabbGBuffer.uploadToGpuScene(gpuVolume);
					break;
				case RenderingTechnique::kDepth:
					m_gpuSceneRenderableAabbDepth.uploadToGpuScene(gpuVolume);
					break;
				case RenderingTechnique::kForward:
					m_gpuSceneRenderableAabbForward.uploadToGpuScene(gpuVolume);
					break;
				case RenderingTechnique::kRtMaterialFetch:
					m_gpuSceneRenderableAabbRt.uploadToGpuScene(gpuVolume);
					break;
				default:
					ANKI_ASSERT(0);
				}
			}
		}

		return;
	}

	// From now on something is dirty, update everything

	updated = true;
	m_anyDirty = false;

	// Sanitize
	m_submeshIdx = min(m_submeshIdx, (m_meshComponent) ? (m_meshComponent->getMeshResource().getSubMeshCount() - 1) : 0);

	// Extract the diffuse color
	Vec3 averageDiffuse(0.0f);
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
				averageDiffuse = diffuseRelatedMtlVar->getValue<ImageResourcePtr>()->getAverageColor().xyz;
			}
			else if(diffuseRelatedMtlVar->getDataType() == ShaderVariableDataType::kVec3)
			{
				averageDiffuse = diffuseRelatedMtlVar->getValue<Vec3>();
			}
			else if(diffuseRelatedMtlVar->getDataType() == ShaderVariableDataType::kU32 && diffuseRelatedMtlVar->tryGetImageResource())
			{
				// Bindless texture
				averageDiffuse = diffuseRelatedMtlVar->tryGetImageResource()->getAverageColor().xyz;
			}
			else
			{
				ANKI_SCENE_LOGW("Couldn't extract a diffuse value for material: %s", mtl.getFilename().cstr());
			}
		}
	}

	// Update the constants
	{
		ConstWeakArray<U8> preallocatedConsts = mtl.getPrefilledLocalConstants();

		if(!m_gpuSceneConstants || m_gpuSceneConstants.getAllocatedSize() != preallocatedConsts.getSizeInBytes())
		{
			GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneConstants);
			m_gpuSceneConstants = GpuSceneBuffer::getSingleton().allocate(preallocatedConsts.getSizeInBytes(), 4);
		}

		GpuSceneMicroPatcher::getSingleton().newCopy(m_gpuSceneConstants.getOffset(), m_gpuSceneConstants.getAllocatedSize(),
													 preallocatedConsts.getBegin());
	}

	// Update renderable
	{
		const MoveComponent& movec = info.m_node->getFirstComponentOfType<MoveComponent>();

		GpuSceneRenderable gpuRenderable = {};
		gpuRenderable.m_worldTransformsIndex = movec.getGpuSceneTransformsIndex();
		gpuRenderable.m_constantsOffset = m_gpuSceneConstants.getOffset();
		gpuRenderable.m_meshLodsIndex =
			(prioritizeEmitter) ? m_emitterComponent->getGpuSceneMeshLodIndex(m_submeshIdx) : m_meshComponent->getGpuSceneMeshLodsIndex(m_submeshIdx);
		gpuRenderable.m_boneTransformsOffset = (m_skinComponent) ? m_skinComponent->getBoneTransformsGpuSceneOffset() : 0;
		gpuRenderable.m_particleEmitterIndex2 = (prioritizeEmitter) ? m_emitterComponent->getGpuSceneParticleEmitter2Index() : kMaxU32;
		if(!!(mtl.getRenderingTechniques() & RenderingTechniqueBit::kRtMaterialFetch))
		{
			const RenderingKey key(RenderingTechnique::kRtMaterialFetch, 0, false, false, false);
			const MaterialVariant& variant = mtl.getOrCreateVariant(key);
			gpuRenderable.m_rtMaterialFetchShaderHandleIndex = variant.getRtShaderGroupHandleIndex();
		}
		gpuRenderable.m_uuid = SceneGraph::getSingleton().getNewUuid();

		const UVec3 u3(averageDiffuse.xyz.clamp(0.0f, 1.0f) * 255.0f);
		gpuRenderable.m_diffuseColor = ((u3.x << 16u) | (u3.y << 8u) | u3.z) & 0xFFFFFFF;
		gpuRenderable.m_sceneNodeUuid = info.m_node->getUuid();

		m_gpuSceneRenderable.uploadToGpuScene(gpuRenderable);
	}

	// Update scene bounds
	{
		const Aabb aabbWorld = computeAabb(*info.m_node);
		info.updateSceneBounds(aabbWorld.getMin().xyz, aabbWorld.getMax().xyz);
	}

	// Update the buckets
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
		key.setSkinned(m_skinComponent != nullptr);
		key.setVelocity(moved);
		key.setMeshletRendering(!prioritizeEmitter
								&& (GrManager::getSingleton().getDeviceCapabilities().m_meshShaders || g_cvarCoreMeshletRendering));

		const MaterialVariant& mvariant = mtl.getOrCreateVariant(key);

		RenderStateInfo state;
		state.m_primitiveTopology = PrimitiveTopology::kTriangles;
		state.m_program = mvariant.getShaderProgram();

		Bool wantsMesletCount = false;
		U32 meshletCount = 0;
		if(!prioritizeEmitter)
		{
			U32 firstIndex, indexCount, firstMeshlet;
			Aabb aabb;
			m_meshComponent->getMeshResource().getSubMeshInfo(0, m_submeshIdx, firstIndex, indexCount, firstMeshlet, meshletCount, aabb);
			wantsMesletCount = key.getMeshletRendering() && !(RenderingTechniqueBit(1 << t) & RenderingTechniqueBit::kAllRt);
		}

		m_renderStateBucketIndices[t] = RenderStateBucketContainer::getSingleton().addUser(state, t, (wantsMesletCount) ? meshletCount : 0);
	}

	// Upload the AABBs to the GPU scene
	{
		const Aabb aabbWorld = computeAabb(*info.m_node);

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
					aabbWorld.getMin().xyz, aabbWorld.getMax().xyz, m_gpuSceneRenderable.getIndex(), m_renderStateBucketIndices[t].get());

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
				initGpuSceneRenderableBoundingVolume(aabbWorld.getMin().xyz, aabbWorld.getMax().xyz, m_gpuSceneRenderable.getIndex(), bucketIdx);

			m_gpuSceneRenderableAabbRt.uploadToGpuScene(gpuVolume);
		}
		else
		{
			m_gpuSceneRenderableAabbRt.free();
		}
	}
}

Error MaterialComponent::serialize(SceneSerializer& serializer)
{
	ANKI_SERIALIZE(m_resource, 1);
	ANKI_SERIALIZE(m_submeshIdx, 1);

	return Error::kNone;
}

} // end namespace anki
