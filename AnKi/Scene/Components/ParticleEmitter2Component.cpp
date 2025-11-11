// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ParticleEmitter2Component.h>
#include <AnKi/Scene/Components/MeshComponent.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Resource/ParticleEmitterResource2.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/MeshResource.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>

namespace anki {

// Contains some shared geometry data and it's a singleton because we don't want to be wasting time and memory for every emitter
class ParticleEmitter2Component::ParticleEmitterQuadGeometry : public MakeSingletonLazyInit<ParticleEmitterQuadGeometry>
{
public:
	UnifiedGeometryBufferAllocation m_quadPositions;
	UnifiedGeometryBufferAllocation m_quadUvs;
	UnifiedGeometryBufferAllocation m_quadIndices;

	GpuSceneArrays::MeshLod::Allocation m_gpuSceneMeshLods;

	U32 m_userCount = 0;
	Bool m_uploadedToGpuScene = false;
	SpinLock m_mtx;

	void init()
	{
		initGeom();
		initGpuScene();
	}

	void initGeom()
	{
		// Allocate and populate a quad
		const U32 vertCount = 4;
		const U32 indexCount = 6;

		m_quadPositions = UnifiedGeometryBuffer::getSingleton().allocateFormat(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition], vertCount);
		m_quadUvs = UnifiedGeometryBuffer::getSingleton().allocateFormat(kMeshRelatedVertexStreamFormats[VertexStreamId::kUv], vertCount);
		m_quadIndices = UnifiedGeometryBuffer::getSingleton().allocateFormat(Format::kR16_Uint, indexCount);

		static_assert(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition] == Format::kR16G16B16A16_Unorm);
		WeakArray<U16Vec4> transientPositions;
		const BufferView positionsAlloc = RebarTransientMemoryPool::getSingleton().allocateCopyBuffer(vertCount, transientPositions);
		transientPositions[0] = U16Vec4(0, 0, 0, 0);
		transientPositions[1] = U16Vec4(kMaxU16, 0, 0, 0);
		transientPositions[2] = U16Vec4(kMaxU16, kMaxU16, 0, 0);
		transientPositions[3] = U16Vec4(0, kMaxU16, 0, 0);

		static_assert(kMeshRelatedVertexStreamFormats[VertexStreamId::kUv] == Format::kR32G32_Sfloat);
		WeakArray<Vec2> transientUvs;
		const BufferView uvsAlloc = RebarTransientMemoryPool::getSingleton().allocateCopyBuffer(vertCount, transientUvs);
		transientUvs[0] = Vec2(0.0f);
		transientUvs[1] = Vec2(1.0f, 0.0f);
		transientUvs[2] = Vec2(1.0f, 1.0f);
		transientUvs[3] = Vec2(0.0f, 1.0f);

		WeakArray<U16> transientIndices;
		const BufferView indicesAlloc = RebarTransientMemoryPool::getSingleton().allocateCopyBuffer(indexCount, transientIndices);
		transientIndices[0] = 0;
		transientIndices[1] = 1;
		transientIndices[2] = 3;
		transientIndices[3] = 1;
		transientIndices[4] = 2;
		transientIndices[5] = 3;

		CommandBufferInitInfo cmdbInit("Particle quad upload");
		cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
		CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);
		Buffer* dstBuff = &UnifiedGeometryBuffer::getSingleton().getBuffer();
		cmdb->copyBufferToBuffer(positionsAlloc, m_quadPositions);
		cmdb->copyBufferToBuffer(uvsAlloc, m_quadUvs);
		cmdb->copyBufferToBuffer(indicesAlloc, m_quadIndices);
		BufferBarrierInfo barrier;
		barrier.m_bufferView = BufferView(dstBuff);
		barrier.m_previousUsage = BufferUsageBit::kCopyDestination;
		barrier.m_nextUsage = dstBuff->getBufferUsage();
		cmdb->setPipelineBarrier({}, {&barrier, 1}, {});
		cmdb->endRecording();

		GrManager::getSingleton().submit(cmdb.get());
	}

	void initGpuScene()
	{
		m_gpuSceneMeshLods.allocate();
	}

	void deinit()
	{
		UnifiedGeometryBuffer::getSingleton().deferredFree(m_quadPositions);
		UnifiedGeometryBuffer::getSingleton().deferredFree(m_quadUvs);
		UnifiedGeometryBuffer::getSingleton().deferredFree(m_quadIndices);

		m_gpuSceneMeshLods.free();
	}

	void addUser()
	{
		LockGuard lock(m_mtx);

		if(m_userCount == 0)
		{
			init();
		}

		++m_userCount;
	}

	void removeUser()
	{
		LockGuard lock(m_mtx);

		ANKI_ASSERT(m_userCount > 0);
		if(m_userCount == 1)
		{
			deinit();
		}

		--m_userCount;
	}

	// Do that outside of init() because we can only upload to the GPU scene at specific places in the frame
	void tryUploadToGpuScene()
	{
		LockGuard lock(m_mtx);

		if(!m_uploadedToGpuScene)
		{
			m_uploadedToGpuScene = true;

			GpuSceneMeshLod meshLod = {};
			meshLod.m_vertexOffsets[U32(VertexStreamId::kPosition)] =
				m_quadPositions.getOffset() / getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition]).m_texelSize;
			meshLod.m_vertexOffsets[U32(VertexStreamId::kUv)] =
				m_quadUvs.getOffset() / getFormatInfo(kMeshRelatedVertexStreamFormats[VertexStreamId::kUv]).m_texelSize;
			meshLod.m_indexCount = 6;
			meshLod.m_firstIndex = m_quadIndices.getOffset() / sizeof(U16);
			meshLod.m_positionScale = 1.0f;
			meshLod.m_positionTranslation = Vec3(-0.5f, -0.5f, 0.0f);

			Array<GpuSceneMeshLod, kMaxLodCount> meshLods;
			meshLods.fill(meshLod);

			m_gpuSceneMeshLods.uploadToGpuScene(meshLods);
		}
	}
};

ParticleEmitter2Component::ParticleEmitter2Component(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	ParticleEmitterQuadGeometry::getSingleton().addUser();
}

ParticleEmitter2Component::~ParticleEmitter2Component()
{
	ParticleEmitterQuadGeometry::getSingleton().removeUser();
}

ParticleEmitter2Component& ParticleEmitter2Component::setParticleEmitterFilename(CString fname)
{
	ParticleEmitterResource2Ptr newRsrc;
	const Error err = ResourceManager::getSingleton().loadResource(fname, newRsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load resource: %s", fname.cstr());
	}
	else
	{
		m_particleEmitterResource = std::move(newRsrc);
		m_anyDirty = true;
	}

	return *this;
}

CString ParticleEmitter2Component::getParticleEmitterFilename() const
{
	if(m_particleEmitterResource)
	{
		return m_particleEmitterResource->getFilename();
	}
	else
	{
		return "*Error*";
	}
}

void ParticleEmitter2Component::onOtherComponentRemovedOrAdded(SceneComponent* other, Bool added)
{
	ANKI_ASSERT(other);

	if(other->getType() == SceneComponentType::kMesh)
	{
		bookkeepComponent(m_meshComponent, other, added);
		m_anyDirty = true;
	}
}

Bool ParticleEmitter2Component::isValid() const
{
	Bool valid = !!m_particleEmitterResource;

	if(m_geomType == ParticleGeometryType::kMeshComponent)
	{
		valid = valid && m_meshComponent->isValid();
	}

	return valid;
}

void ParticleEmitter2Component::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	m_gpuSceneReallocationsThisFrame = false;
	m_dt = F32(info.m_dt);

	if(!isValid()) [[unlikely]]
	{
		for(auto& s : m_gpuScene.m_particleStreams)
		{
			GpuSceneBuffer::getSingleton().deferredFree(s);
		}
		GpuSceneBuffer::getSingleton().deferredFree(m_gpuScene.m_aliveParticleIndices);
		GpuSceneBuffer::getSingleton().deferredFree(m_gpuScene.m_anKiParticleEmitterProperties);
		m_gpuScene.m_gpuSceneParticleEmitter.free();
		m_gpuSceneReallocationsThisFrame = true;
		return;
	}

	ParticleEmitterQuadGeometry::getSingleton().tryUploadToGpuScene();

	if(info.m_checkForResourceUpdates) [[unlikely]]
	{
		if(m_particleEmitterResource->isObsolete()) [[unlikely]]
		{
			ANKI_SCENE_LOGV("Particle emitter resource is obsolete. Will reload it");
			BaseString<MemoryPoolPtrWrapper<StackMemoryPool>> fname(info.m_framePool);
			fname = m_particleEmitterResource->getFilename();
			ParticleEmitterResource2Ptr oldVersion = m_particleEmitterResource;
			m_particleEmitterResource.reset(nullptr);
			if(ResourceManager::getSingleton().loadResource(fname, m_particleEmitterResource))
			{
				ANKI_SCENE_LOGE("Can't update the particle resource. Ignoring the load");
				m_particleEmitterResource = oldVersion;
			}
			else
			{
				m_anyDirty = true;
				ANKI_ASSERT(!m_particleEmitterResource->isObsolete());
			}
		}
	}

	if(!m_anyDirty) [[likely]]
	{
		return;
	}

	// From now on it's dirty, do all updates

	m_gpuSceneReallocationsThisFrame = true; // Difficult to properly track so set it to true and don't bother
	m_anyDirty = false;
	updated = true;
	const ParticleEmitterResourceCommonProperties& commonProps = m_particleEmitterResource->getCommonProperties();

	// Streams
	for(ParticleProperty prop : EnumIterable<ParticleProperty>())
	{
		GpuSceneBuffer::getSingleton().deferredFree(m_gpuScene.m_particleStreams[prop]);
		m_gpuScene.m_particleStreams[prop] =
			GpuSceneBuffer::getSingleton().allocate(commonProps.m_particleCount * kParticlePropertySize[prop], alignof(U32));
	}

	// Alive particles index buffer
	{
		GpuSceneBuffer::getSingleton().deferredFree(m_gpuScene.m_aliveParticleIndices);
		m_gpuScene.m_aliveParticleIndices = GpuSceneBuffer::getSingleton().allocate(commonProps.m_particleCount * sizeof(U32), alignof(U32));
	}

	// AnKiParticleEmitterProperties
	{
		GpuSceneBuffer::getSingleton().deferredFree(m_gpuScene.m_anKiParticleEmitterProperties);

		ConstWeakArray<U8> prefilled = m_particleEmitterResource->getPrefilledAnKiParticleEmitterProperties();
		m_gpuScene.m_anKiParticleEmitterProperties = GpuSceneBuffer::getSingleton().allocate(prefilled.getSizeInBytes(), alignof(U32));

		GpuSceneMicroPatcher::getSingleton().newCopy(m_gpuScene.m_anKiParticleEmitterProperties, prefilled.getSizeInBytes(), prefilled.getBegin());
	}

	// GpuSceneParticleEmitter2
	{
		if(!m_gpuScene.m_gpuSceneParticleEmitter)
		{
			m_gpuScene.m_gpuSceneParticleEmitter.allocate();
		}

		GpuSceneParticleEmitter2 emitter;
		zeroMemory(emitter);

		for(ParticleProperty prop : EnumIterable<ParticleProperty>())
		{
			emitter.m_particleStateSteamOffsets[U32(prop)] = m_gpuScene.m_particleStreams[prop].getOffset();
		}
		emitter.m_aliveParticleIndicesOffset = m_gpuScene.m_aliveParticleIndices.getOffset();
		emitter.m_particleCount = commonProps.m_particleCount;

		emitter.m_emissionPeriod = commonProps.m_emissionPeriod;
		emitter.m_particlesPerEmission = commonProps.m_particlesPerEmission;
		emitter.m_particleEmitterPropertiesOffset = m_gpuScene.m_anKiParticleEmitterProperties.getOffset();

		if(m_geomType == ParticleGeometryType::kMeshComponent)
		{
			const MeshResource& meshr = m_meshComponent->getMeshResource();
			emitter.m_particleAabbMin = meshr.getBoundingShape().getMin().xyz();
			emitter.m_particleAabbMax = meshr.getBoundingShape().getMax().xyz();
		}
		else
		{
			emitter.m_particleAabbMin = Vec3(-0.5f);
			emitter.m_particleAabbMax = Vec3(+0.5f);
		}

		emitter.m_reinitializeOnNextUpdate = 1;
		emitter.m_worldTransformsIndex = info.m_node->getFirstComponentOfType<MoveComponent>().getGpuSceneTransformsIndex();
		emitter.m_uuid = getUuid();

		m_gpuScene.m_gpuSceneParticleEmitter.uploadToGpuScene(emitter);
	}
}

U32 ParticleEmitter2Component::getGpuSceneMeshLodIndex(U32 submeshIdx) const
{
	ANKI_ASSERT(isValid());
	if(m_geomType == ParticleGeometryType::kQuad)
	{
		return ParticleEmitterQuadGeometry::getSingleton().m_gpuSceneMeshLods.getIndex() * kMaxLodCount;
	}
	else
	{
		return m_meshComponent->getGpuSceneMeshLodsIndex(submeshIdx);
	}
}

} // end namespace anki
