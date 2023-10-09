// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/ParticleEmitterComponent.h>
#include <AnKi/Scene/SceneGraph.h>
#include <AnKi/Scene/SceneNode.h>
#include <AnKi/Scene/Components/MoveComponent.h>
#include <AnKi/Resource/ParticleEmitterResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Physics/PhysicsBody.h>
#include <AnKi/Physics/PhysicsCollisionShape.h>
#include <AnKi/Physics/PhysicsWorld.h>
#include <AnKi/Math.h>
#include <AnKi/Shaders/Include/GpuSceneFunctions.h>
#include <AnKi/Core/GpuMemory/RebarTransientMemoryPool.h>

namespace anki {

static Vec3 getRandom(const Vec3& min, const Vec3& max)
{
	Vec3 out;
	out.x() = mix(min.x(), max.x(), getRandomRange(0.0f, 1.0f));
	out.y() = mix(min.y(), max.y(), getRandomRange(0.0f, 1.0f));
	out.z() = mix(min.z(), max.z(), getRandomRange(0.0f, 1.0f));
	return out;
}

/// Particle base
class ParticleEmitterComponent::ParticleBase
{
public:
	Second m_timeOfBirth; ///< Keep the time of birth for nice effects
	Second m_timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead

	F32 m_initialSize;
	F32 m_finalSize;
	F32 m_crntSize;

	F32 m_initialAlpha;
	F32 m_finalAlpha;
	F32 m_crntAlpha;

	Vec3 m_crntPosition;

	Bool isDead() const
	{
		return m_timeOfDeath < 0.0;
	}

	/// Kill the particle
	void killCommon()
	{
		ANKI_ASSERT(m_timeOfDeath > 0.0);
		m_timeOfDeath = -1.0;
	}

	/// Revive the particle
	void reviveCommon(const ParticleEmitterProperties& props, Second crntTime)
	{
		ANKI_ASSERT(isDead());

		// life
		m_timeOfDeath = crntTime + getRandomRange(props.m_particle.m_minLife, props.m_particle.m_maxLife);
		m_timeOfBirth = crntTime;

		// Size
		m_initialSize = getRandomRange(props.m_particle.m_minInitialSize, props.m_particle.m_maxInitialSize);
		m_finalSize = getRandomRange(props.m_particle.m_minFinalSize, props.m_particle.m_maxFinalSize);

		// Alpha
		m_initialAlpha = getRandomRange(props.m_particle.m_minInitialAlpha, props.m_particle.m_maxInitialAlpha);
		m_finalAlpha = getRandomRange(props.m_particle.m_minFinalAlpha, props.m_particle.m_maxFinalAlpha);
	}

	/// Common sumulation code
	void simulateCommon(Second crntTime)
	{
		const F32 lifeFactor = F32((crntTime - m_timeOfBirth) / (m_timeOfDeath - m_timeOfBirth));

		m_crntSize = mix(m_initialSize, m_finalSize, lifeFactor);
		m_crntAlpha = mix(m_initialAlpha, m_finalAlpha, lifeFactor);
	}
};

/// Simple particle for simple simulation
class ParticleEmitterComponent::SimpleParticle : public ParticleEmitterComponent::ParticleBase
{
public:
	Vec3 m_velocity = Vec3(0.0f);
	Vec3 m_acceleration = Vec3(0.0f);

	void kill()
	{
		killCommon();
	}

	void revive(const ParticleEmitterProperties& props, const Transform& trf, Second crntTime)
	{
		reviveCommon(props, crntTime);
		m_velocity = Vec3(0.0f);

		m_acceleration = getRandom(props.m_particle.m_minGravity, props.m_particle.m_maxGravity);

		// Set the initial position
		m_crntPosition = getRandom(props.m_particle.m_minStartingPosition, props.m_particle.m_maxStartingPosition);
		m_crntPosition += trf.getOrigin().xyz();
	}

	void simulate(Second prevUpdateTime, Second crntTime)
	{
		simulateCommon(crntTime);

		const F32 dt = F32(crntTime - prevUpdateTime);

		const Vec3 xp = m_crntPosition;
		const Vec3 xc = m_acceleration * (dt * dt) + m_velocity * dt + xp;

		m_crntPosition = xc;

		m_velocity += m_acceleration * dt;
	}
};

/// Particle for bullet simulations
class ParticleEmitterComponent::PhysicsParticle : public ParticleEmitterComponent::ParticleBase
{
public:
	PhysicsBodyPtr m_body;

	PhysicsParticle(const PhysicsBodyInitInfo& init, ParticleEmitterComponent* component)
	{
		m_body = PhysicsWorld::getSingleton().newInstance<PhysicsBody>(init);
		m_body->setUserData(component);
		m_body->activate(false);
		m_body->setMaterialGroup(PhysicsMaterialBit::kParticle);
		m_body->setMaterialMask(PhysicsMaterialBit::kStaticGeometry);
		m_body->setAngularFactor(Vec3(0.0f));
	}

	void kill()
	{
		killCommon();
		m_body->activate(false);
	}

	void revive(const ParticleEmitterProperties& props, const Transform& trf, Second crntTime)
	{
		reviveCommon(props, crntTime);

		// pre calculate
		const Bool forceFlag = props.forceEnabled();
		const Bool worldGravFlag = props.wordGravityEnabled();

		// Activate it
		m_body->activate(true);
		m_body->setLinearVelocity(Vec3(0.0f));
		m_body->setAngularVelocity(Vec3(0.0f));
		m_body->clearForces();

		// force
		if(forceFlag)
		{
			Vec3 forceDir = getRandom(props.m_particle.m_minForceDirection, props.m_particle.m_maxForceDirection);
			forceDir.normalize();

			// The forceDir depends on the particle emitter rotation
			forceDir = trf.getRotation().getRotationPart() * forceDir;

			const F32 forceMag = getRandomRange(props.m_particle.m_minForceMagnitude, props.m_particle.m_maxForceMagnitude);
			m_body->applyForce(forceDir * forceMag, Vec3(0.0f));
		}

		// gravity
		if(!worldGravFlag)
		{
			m_body->setGravity(getRandom(props.m_particle.m_minGravity, props.m_particle.m_maxGravity));
		}

		// Starting pos. In local space
		Vec3 pos = getRandom(props.m_particle.m_minStartingPosition, props.m_particle.m_maxStartingPosition);
		pos = trf.transform(pos);

		m_body->setTransform(Transform(pos.xyz0(), trf.getRotation(), 1.0f));
		m_crntPosition = pos;
	}

	void simulate([[maybe_unused]] Second prevUpdateTime, Second crntTime)
	{
		simulateCommon(crntTime);
		m_crntPosition = m_body->getTransform().getOrigin().xyz();
	}
};

ParticleEmitterComponent::ParticleEmitterComponent(SceneNode* node)
	: SceneComponent(node, kClassType)
{
	// Allocate and populate a quad
	const U32 vertCount = 4;
	const U32 indexCount = 6;

	m_quadPositions = UnifiedGeometryBuffer::getSingleton().allocateFormat(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition], vertCount);
	m_quadUvs = UnifiedGeometryBuffer::getSingleton().allocateFormat(kMeshRelatedVertexStreamFormats[VertexStreamId::kUv], vertCount);
	m_quadIndices = UnifiedGeometryBuffer::getSingleton().allocateFormat(Format::kR16_Uint, indexCount);

	static_assert(kMeshRelatedVertexStreamFormats[VertexStreamId::kPosition] == Format::kR16G16B16A16_Unorm);
	WeakArray<U16Vec4> transientPositions;
	const RebarAllocation positionsAlloc = RebarTransientMemoryPool::getSingleton().allocateFrame(vertCount, transientPositions);
	transientPositions[0] = U16Vec4(0, 0, 0, 0);
	transientPositions[1] = U16Vec4(kMaxU16, 0, 0, 0);
	transientPositions[2] = U16Vec4(kMaxU16, kMaxU16, 0, 0);
	transientPositions[3] = U16Vec4(0, kMaxU16, 0, 0);

	static_assert(kMeshRelatedVertexStreamFormats[VertexStreamId::kUv] == Format::kR32G32_Sfloat);
	WeakArray<Vec2> transientUvs;
	const RebarAllocation uvsAlloc = RebarTransientMemoryPool::getSingleton().allocateFrame(vertCount, transientUvs);
	transientUvs[0] = Vec2(0.0f);
	transientUvs[1] = Vec2(1.0f, 0.0f);
	transientUvs[2] = Vec2(1.0f, 1.0f);
	transientUvs[3] = Vec2(0.0f, 1.0f);

	WeakArray<U16> transientIndices;
	const RebarAllocation indicesAlloc = RebarTransientMemoryPool::getSingleton().allocateFrame(indexCount, transientIndices);
	transientIndices[0] = 0;
	transientIndices[1] = 1;
	transientIndices[2] = 3;
	transientIndices[3] = 1;
	transientIndices[4] = 2;
	transientIndices[5] = 3;

	CommandBufferInitInfo cmdbInit("Particle quad upload");
	cmdbInit.m_flags |= CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);
	Buffer* srcBuff = &RebarTransientMemoryPool::getSingleton().getBuffer();
	Buffer* dstBuff = &UnifiedGeometryBuffer::getSingleton().getBuffer();
	cmdb->copyBufferToBuffer(srcBuff, positionsAlloc.getOffset(), dstBuff, m_quadPositions.getOffset(), positionsAlloc.getRange());
	cmdb->copyBufferToBuffer(srcBuff, uvsAlloc.getOffset(), dstBuff, m_quadUvs.getOffset(), uvsAlloc.getRange());
	cmdb->copyBufferToBuffer(srcBuff, indicesAlloc.getOffset(), dstBuff, m_quadIndices.getOffset(), indicesAlloc.getRange());
	BufferBarrierInfo barrier;
	barrier.m_buffer = dstBuff;
	barrier.m_offset = 0;
	barrier.m_range = kMaxPtrSize;
	barrier.m_previousUsage = BufferUsageBit::kTransferDestination;
	barrier.m_nextUsage = dstBuff->getBufferUsage();
	cmdb->setPipelineBarrier({}, {&barrier, 1}, {});
	cmdb->flush();
}

ParticleEmitterComponent::~ParticleEmitterComponent()
{
}

void ParticleEmitterComponent::loadParticleEmitterResource(CString filename)
{
	// Load
	ParticleEmitterResourcePtr rsrc;
	const Error err = ResourceManager::getSingleton().loadResource(filename, rsrc);
	if(err)
	{
		ANKI_SCENE_LOGE("Failed to load particle emitter");
		return;
	}

	m_particleEmitterResource = std::move(rsrc);

	m_props = m_particleEmitterResource->getProperties();
	m_resourceUpdated = true;

	// Cleanup
	m_simpleParticles.destroy();
	m_physicsParticles.destroy();
	GpuSceneBuffer::getSingleton().deferredFree(m_gpuScenePositions);
	GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneScales);
	GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneAlphas);
	GpuSceneBuffer::getSingleton().deferredFree(m_gpuSceneUniforms);

	for(RenderStateBucketIndex& idx : m_renderStateBuckets)
	{
		RenderStateBucketContainer::getSingleton().removeUser(idx);
	}

	// Init particles
	m_simulationType = (m_props.m_usePhysicsEngine) ? SimulationType::kPhysicsEngine : SimulationType::kSimple;
	if(m_simulationType == SimulationType::kPhysicsEngine)
	{
		PhysicsCollisionShapePtr collisionShape = PhysicsWorld::getSingleton().newInstance<PhysicsSphere>(m_props.m_particle.m_minInitialSize / 2.0f);

		PhysicsBodyInitInfo binit;
		binit.m_shape = std::move(collisionShape);

		m_physicsParticles.resizeStorage(m_props.m_maxNumOfParticles);
		for(U32 i = 0; i < m_props.m_maxNumOfParticles; i++)
		{
			binit.m_mass = getRandomRange(m_props.m_particle.m_minMass, m_props.m_particle.m_maxMass);
			m_physicsParticles.emplaceBack(binit, this);
		}
	}
	else
	{
		m_simpleParticles.resize(m_props.m_maxNumOfParticles);
	}

	// GPU scene allocations
	m_gpuScenePositions = GpuSceneBuffer::getSingleton().allocate(sizeof(Vec3) * m_props.m_maxNumOfParticles, alignof(F32));
	m_gpuSceneAlphas = GpuSceneBuffer::getSingleton().allocate(sizeof(F32) * m_props.m_maxNumOfParticles, alignof(F32));
	m_gpuSceneScales = GpuSceneBuffer::getSingleton().allocate(sizeof(F32) * m_props.m_maxNumOfParticles, alignof(F32));
	m_gpuSceneUniforms =
		GpuSceneBuffer::getSingleton().allocate(m_particleEmitterResource->getMaterial()->getPrefilledLocalUniforms().getSizeInBytes(), alignof(U32));

	// Allocate buckets
	for(RenderingTechnique t :
		EnumBitsIterable<RenderingTechnique, RenderingTechniqueBit>(m_particleEmitterResource->getMaterial()->getRenderingTechniques()))
	{
		RenderingKey key;
		key.setRenderingTechnique(t);
		ShaderProgramPtr prog;
		m_particleEmitterResource->getRenderingInfo(key, prog);

		RenderStateInfo state;
		state.m_program = prog;
		state.m_primitiveTopology = PrimitiveTopology::kTriangles;
		state.m_indexedDrawcall = false;
		m_renderStateBuckets[t] = RenderStateBucketContainer::getSingleton().addUser(state, t);
	}
}

Error ParticleEmitterComponent::update(SceneComponentUpdateInfo& info, Bool& updated)
{
	if(!m_particleEmitterResource.isCreated()) [[unlikely]]
	{
		updated = false;
		return Error::kNone;
	}

	updated = true;
	Vec3* positions;
	F32* scales;
	F32* alphas;

	Aabb aabbWorld;
	if(m_simulationType == SimulationType::kSimple)
	{
		simulate(info.m_previousTime, info.m_currentTime, info.m_node->getWorldTransform(), WeakArray<SimpleParticle>(m_simpleParticles), positions,
				 scales, alphas, aabbWorld);
	}
	else
	{
		ANKI_ASSERT(m_simulationType == SimulationType::kPhysicsEngine);
		simulate(info.m_previousTime, info.m_currentTime, info.m_node->getWorldTransform(), WeakArray<PhysicsParticle>(m_physicsParticles), positions,
				 scales, alphas, aabbWorld);
	}

	// Upload particles to the GPU scene
	GpuSceneMicroPatcher& patcher = GpuSceneMicroPatcher::getSingleton();
	if(m_aliveParticleCount > 0)
	{
		patcher.newCopy(*info.m_framePool, m_gpuScenePositions, sizeof(Vec3) * m_aliveParticleCount, positions);
		patcher.newCopy(*info.m_framePool, m_gpuSceneScales, sizeof(F32) * m_aliveParticleCount, scales);
		patcher.newCopy(*info.m_framePool, m_gpuSceneAlphas, sizeof(F32) * m_aliveParticleCount, alphas);
	}

	if(m_resourceUpdated)
	{
		// Upload GpuSceneParticleEmitter
		GpuSceneParticleEmitter particles = {};
		particles.m_vertexOffsets[U32(VertexStreamId::kParticlePosition)] = m_gpuScenePositions.getOffset();
		particles.m_vertexOffsets[U32(VertexStreamId::kParticleColor)] = m_gpuSceneAlphas.getOffset();
		particles.m_vertexOffsets[U32(VertexStreamId::kParticleScale)] = m_gpuSceneScales.getOffset();
		particles.m_aliveParticleCount = m_aliveParticleCount;
		if(!m_gpuSceneParticleEmitter.isValid())
		{
			m_gpuSceneParticleEmitter.allocate();
		}
		m_gpuSceneParticleEmitter.uploadToGpuScene(particles);

		// Upload uniforms
		patcher.newCopy(*info.m_framePool, m_gpuSceneUniforms, m_particleEmitterResource->getMaterial()->getPrefilledLocalUniforms().getSizeInBytes(),
						m_particleEmitterResource->getMaterial()->getPrefilledLocalUniforms().getBegin());

		// Upload mesh LODs
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
		if(!m_gpuSceneMeshLods.isValid())
		{
			m_gpuSceneMeshLods.allocate();
		}
		m_gpuSceneMeshLods.uploadToGpuScene(meshLods);

		// Upload the GpuSceneRenderable
		GpuSceneRenderable renderable;
		renderable.m_boneTransformsOffset = 0;
		renderable.m_constantsOffset = m_gpuSceneUniforms.getOffset();
		renderable.m_meshLodsOffset = m_gpuSceneMeshLods.getGpuSceneOffset();
		renderable.m_particleEmitterOffset = m_gpuSceneParticleEmitter.getGpuSceneOffset();
		renderable.m_worldTransformsOffset = 0;
		renderable.m_uuid = SceneGraph::getSingleton().getNewUuid();
		if(!m_gpuSceneRenderable.isValid())
		{
			m_gpuSceneRenderable.allocate();
		}
		m_gpuSceneRenderable.uploadToGpuScene(renderable);
	}

	if(!m_resourceUpdated)
	{
		// Always upload GpuSceneParticleEmitter

		GpuSceneParticleEmitter particles = {};
		particles.m_vertexOffsets[U32(VertexStreamId::kParticlePosition)] = m_gpuScenePositions.getOffset();
		particles.m_vertexOffsets[U32(VertexStreamId::kParticleColor)] = m_gpuSceneAlphas.getOffset();
		particles.m_vertexOffsets[U32(VertexStreamId::kParticleScale)] = m_gpuSceneScales.getOffset();
		particles.m_aliveParticleCount = m_aliveParticleCount;
		if(!m_gpuSceneParticleEmitter.isValid())
		{
			m_gpuSceneParticleEmitter.allocate();
		}
		m_gpuSceneParticleEmitter.uploadToGpuScene(particles);
	}

	// Upload the GpuSceneRenderableBoundingVolume always
	for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
	{
		if(!!(RenderingTechniqueBit(1 << t) & m_particleEmitterResource->getMaterial()->getRenderingTechniques()))
		{
			const GpuSceneRenderableBoundingVolume gpuVolume = initGpuSceneRenderableBoundingVolume(
				aabbWorld.getMin().xyz(), aabbWorld.getMax().xyz(), m_gpuSceneRenderable.getIndex(), m_renderStateBuckets[t].get());
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
		else if(!!(RenderingTechniqueBit(1 << t) & RenderingTechniqueBit::kAllRt))
		{
			continue;
		}
		else
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
	}

	m_resourceUpdated = false;
	return Error::kNone;
}

template<typename TParticle>
void ParticleEmitterComponent::simulate(Second prevUpdateTime, Second crntTime, const Transform& worldTransform, WeakArray<TParticle> particles,
										Vec3*& positions, F32*& scales, F32*& alphas, Aabb& aabbWorld)
{
	// - Deactivate the dead particles
	// - Calc the AABB
	// - Calc the instancing stuff

	Vec3 aabbMin(kMaxF32);
	Vec3 aabbMax(kMinF32);
	m_aliveParticleCount = 0;

	positions =
		static_cast<Vec3*>(SceneGraph::getSingleton().getFrameMemoryPool().allocate(m_props.m_maxNumOfParticles * sizeof(Vec3), alignof(Vec3)));
	scales = static_cast<F32*>(SceneGraph::getSingleton().getFrameMemoryPool().allocate(m_props.m_maxNumOfParticles * sizeof(F32), alignof(F32)));
	alphas = static_cast<F32*>(SceneGraph::getSingleton().getFrameMemoryPool().allocate(m_props.m_maxNumOfParticles * sizeof(F32), alignof(F32)));

	F32 maxParticleSize = -1.0f;

	for(TParticle& particle : particles)
	{
		if(particle.isDead())
		{
			// if its already dead so dont deactivate it again
			continue;
		}

		if(particle.m_timeOfDeath < crntTime)
		{
			// Just died
			particle.kill();
		}
		else
		{
			// It's alive

			// This will calculate a new world transformation
			particle.simulate(prevUpdateTime, crntTime);

			const Vec3& origin = particle.m_crntPosition;

			aabbMin = aabbMin.min(origin);
			aabbMax = aabbMax.max(origin);

			positions[m_aliveParticleCount] = origin;

			scales[m_aliveParticleCount] = particle.m_crntSize;
			maxParticleSize = max(maxParticleSize, particle.m_crntSize);

			alphas[m_aliveParticleCount] = clamp(particle.m_crntAlpha, 0.0f, 1.0f);

			++m_aliveParticleCount;
		}
	}

	// AABB
	if(m_aliveParticleCount != 0)
	{
		ANKI_ASSERT(maxParticleSize > 0.0f);
		const Vec3 min = aabbMin - maxParticleSize;
		const Vec3 max = aabbMax + maxParticleSize;
		aabbWorld = Aabb(min, max);
	}
	else
	{
		aabbWorld = Aabb(Vec3(0.0f), Vec3(0.001f));
		positions = nullptr;
		alphas = scales = nullptr;
	}

	//
	// Emit new particles
	//
	if(m_timeLeftForNextEmission <= 0.0)
	{
		U particleCount = 0; // How many particles I am allowed to emmit
		for(TParticle& particle : particles)
		{
			if(!particle.isDead())
			{
				// its alive so skip it
				continue;
			}

			particle.revive(m_props, worldTransform, crntTime);

			// do the rest
			++particleCount;
			if(particleCount >= m_props.m_particlesPerEmission)
			{
				break;
			}
		} // end for all particles

		m_timeLeftForNextEmission = m_props.m_emissionPeriod;
	} // end if can emit
	else
	{
		m_timeLeftForNextEmission -= crntTime - prevUpdateTime;
	}
}

} // end namespace anki
