// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/MaterialComponent.h>
#include <AnKi/Scene/Components/SkinComponent.h>
#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Core/App.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>

namespace anki {

MaterialComponent::MaterialComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	m_gpuSceneTransforms.allocate();
	m_gpuSceneRenderable.allocate();
	m_gpuSceneMeshLods.allocate();
}

MaterialComponent::~MaterialComponent()
{
	m_gpuSceneTransforms.free();
	m_gpuSceneRenderable.free();
	m_gpuSceneMeshLods.free();
}

MaterialComponent& MaterialComponent::setMaterialFilename(CString fname)
{
	MaterialResourcePtr newRsrc;
	const Error err = ResourceManager::getSingleton().loadResource(fname, newRsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load resource: %s", fname.cstr());
	}
	else
	{
		m_resource = std::move(newRsrc);
		m_castsShadow = m_resource->castsShadow();
		m_resourceDirty = true;
	}

	return *this;
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

	if(other->getType() == SceneComponentType::kSkin)
	{
		const Bool alreadyHasSkinComponent = m_skinComponent != nullptr;
		if(added && !alreadyHasSkinComponent)
		{
			m_skinComponent = static_cast<SkinComponent*>(other);
			m_skinDirty = true;
		}
		else if(!added && other == m_skinComponent)
		{
			m_skinComponent = nullptr;
			m_skinDirty = true;
		}
	}

	if(other->getType() == SceneComponentType::kMesh)
	{
		const Bool alreadyHasMeshComponent = m_meshComponent != nullptr;
		if(added && !alreadyHasMeshComponent)
		{
			m_meshComponent = static_cast<MeshComponent*>(other);
			m_meshComponentDirty = true;
		}
		else if(!added && other == m_meshComponent)
		{
			m_meshComponent = nullptr;
			m_meshComponentDirty = true;
		}
	}
}

Aabb MaterialComponent::computeAabb(U32 submeshIndex, const SceneNode& node) const
{
	U32 firstIndex, indexCount, firstMeshlet, meshletCount;
	Aabb aabbLocal;
	m_meshComponent->getMeshResource().getSubMeshInfo(0, submeshIndex, firstIndex, indexCount, firstMeshlet, meshletCount, aabbLocal);

	if(m_skinComponent)
	{
		aabbLocal = m_skinComponent->getBoneBoundingVolumeLocalSpace().getCompoundShape(aabbLocal);
	}

	const Aabb aabbWorld = aabbLocal.getTransformed(node.getWorldTransform());
	return aabbWorld;
}

void MaterialComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	const Bool mtlUpdated = m_resourceDirty;
	const Bool meshUpdated = m_meshComponentDirty || (m_meshComponent && m_meshComponent->updatedThisFrame());
	const Bool moved = info.m_node->movedThisFrame() || m_firstTimeUpdate;
	const Bool movedLastFrame = m_movedLastFrame || m_firstTimeUpdate;
	const Bool skinUpdated = m_skinDirty;
	const Bool submeshUpdated = m_submeshIdxDirty;
	const Bool hasSkin = m_skinComponent && m_skinComponent->isEnabled();
	const Bool isValid = m_resource.isCreated() && m_meshComponent && m_meshComponent->isEnabled();
	m_resourceDirty = false;
	m_firstTimeUpdate = false;
	m_meshComponentDirty = false;
	m_movedLastFrame = moved;
	m_skinDirty = false;
	m_submeshIdxDirty = false;

	updated = mtlUpdated || meshUpdated || moved || skinUpdated || submeshUpdated;

	if(!isValid) [[unlikely]]
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

	const MaterialResource& mtl = *m_resource;
	const MeshResource& mesh = m_meshComponent->getMeshResource();
	const U32 submeshIdx = min(mesh.getSubMeshCount() - 1, m_submeshIdx);

	// Extract the diffuse color
	Vec3 averageDiffuse(0.0f);
	if(mtlUpdated)
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

	// Upload transforms
	if(moved || movedLastFrame) [[unlikely]]
	{
		Array<Mat3x4, 2> trfs;
		trfs[0] = Mat3x4(info.m_node->getWorldTransform());
		trfs[1] = Mat3x4(info.m_node->getPreviousWorldTransform());
		m_gpuSceneTransforms.uploadToGpuScene(trfs);
	}

	// Update mesh lods
	const Bool meshLodsNeedUpdate = meshUpdated || submeshUpdated;
	if(meshLodsNeedUpdate) [[unlikely]]
	{
		Array<GpuSceneMeshLod, kMaxLodCount> meshLods;

		for(U32 l = 0; l < mesh.getLodCount(); ++l)
		{
			GpuSceneMeshLod& meshLod = meshLods[l];
			meshLod = {};
			meshLod.m_positionScale = mesh.getPositionsScale();
			meshLod.m_positionTranslation = mesh.getPositionsTranslation();

			U32 firstIndex, indexCount, firstMeshlet, meshletCount;
			Aabb aabb;
			mesh.getSubMeshInfo(l, submeshIdx, firstIndex, indexCount, firstMeshlet, meshletCount, aabb);

			U32 totalIndexCount;
			IndexType indexType;
			PtrSize indexUgbOffset;
			mesh.getIndexBufferInfo(l, indexUgbOffset, totalIndexCount, indexType);

			for(VertexStreamId stream = VertexStreamId::kMeshRelatedFirst; stream < VertexStreamId::kMeshRelatedCount; ++stream)
			{
				if(mesh.isVertexStreamPresent(stream))
				{
					U32 vertCount;
					PtrSize ugbOffset;
					mesh.getVertexBufferInfo(l, stream, ugbOffset, vertCount);
					const PtrSize elementSize = getFormatInfo(kMeshRelatedVertexStreamFormats[stream]).m_texelSize;
					ANKI_ASSERT(ugbOffset % elementSize == 0);
					meshLod.m_vertexOffsets[U32(stream)] = U32(ugbOffset / elementSize);
				}
				else
				{
					meshLod.m_vertexOffsets[U32(stream)] = kMaxU32;
				}
			}

			meshLod.m_indexCount = indexCount;

			ANKI_ASSERT(indexUgbOffset % getIndexSize(indexType) == 0);
			meshLod.m_firstIndex = U32(indexUgbOffset / getIndexSize(indexType)) + firstIndex;

			meshLod.m_renderableIndex = m_gpuSceneRenderable.getIndex();

			if(GrManager::getSingleton().getDeviceCapabilities().m_meshShaders || g_cvarCoreMeshletRendering)
			{
				U32 dummy;
				PtrSize meshletBoundingVolumesUgbOffset, meshletGometryDescriptorsUgbOffset;
				mesh.getMeshletBufferInfo(l, meshletBoundingVolumesUgbOffset, meshletGometryDescriptorsUgbOffset, dummy);

				meshLod.m_firstMeshletBoundingVolume = firstMeshlet + U32(meshletBoundingVolumesUgbOffset / sizeof(MeshletBoundingVolume));
				meshLod.m_firstMeshletGeometryDescriptor = firstMeshlet + U32(meshletGometryDescriptorsUgbOffset / sizeof(MeshletGeometryDescriptor));
				meshLod.m_meshletCount = meshletCount;
			}

			meshLod.m_lod = l;

			if(!!(mtl.getRenderingTechniques() & RenderingTechniqueBit::kAllRt))
			{
				const U64 address = mesh.getBottomLevelAccelerationStructure(l, submeshIdx)->getGpuAddress();
				memcpy(&meshLod.m_blasAddress, &address, sizeof(meshLod.m_blasAddress));

				meshLod.m_tlasInstanceMask = 0xFFFFFFFF;
			}
		}

		// Copy the last LOD to the rest just in case
		for(U32 l = mesh.getLodCount(); l < kMaxLodCount; ++l)
		{
			meshLods[l] = meshLods[l - 1];
		}

		m_gpuSceneMeshLods.uploadToGpuScene(meshLods);
	}

	// Update the constants
	const Bool constantsNeedUpdate = mtlUpdated;
	if(mtlUpdated) [[unlikely]]
	{
		ConstWeakArray<U8> preallocatedConsts = mtl.getPrefilledLocalConstants();

		if(!m_gpuSceneConstants.isValid() || m_gpuSceneConstants.getAllocatedSize() != preallocatedConsts.getSizeInBytes())
		{
			GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneConstants);
			m_gpuSceneConstants = GpuSceneBuffer::getSingleton().allocate(preallocatedConsts.getSizeInBytes(), 4);
		}

		GpuSceneMicroPatcher::getSingleton().newCopy(*info.m_framePool, m_gpuSceneConstants.getOffset(), m_gpuSceneConstants.getAllocatedSize(),
													 preallocatedConsts.getBegin());
	}

	// Update renderable
	if(constantsNeedUpdate || skinUpdated) [[unlikely]]
	{
		GpuSceneRenderable gpuRenderable = {};
		gpuRenderable.m_worldTransformsIndex = m_gpuSceneTransforms.getIndex() * 2;
		gpuRenderable.m_constantsOffset = m_gpuSceneConstants.getOffset();
		gpuRenderable.m_meshLodsIndex = m_gpuSceneMeshLods.getIndex() * kMaxLodCount;
		gpuRenderable.m_boneTransformsOffset = (hasSkin) ? m_skinComponent->getBoneTransformsGpuSceneOffset() : 0;
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
	const Bool aabbUpdated = moved || meshUpdated || submeshUpdated || hasSkin;
	if(aabbUpdated) [[unlikely]]
	{
		const Aabb aabbWorld = computeAabb(submeshIdx, *info.m_node);
		SceneGraph::getSingleton().updateSceneBounds(aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz());
	}

	// Update the buckets
	const Bool bucketsNeedUpdate = mtlUpdated || submeshUpdated || moved != movedLastFrame;
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
			state.m_indexedDrawcall = true;
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
