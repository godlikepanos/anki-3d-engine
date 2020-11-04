// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/GpuParticleEmitterNode.h>
#include <anki/scene/SceneGraph.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/GenericGpuComputeJobComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/resource/ResourceManager.h>
#include <anki/shaders/include/ParticleTypes.h>

namespace anki
{

class GpuParticleEmitterNode::MoveFeedbackComponent : public SceneComponent
{
public:
	MoveFeedbackComponent()
		: SceneComponent(SceneComponentType::NONE)
	{
	}

	ANKI_USE_RESULT Error update(SceneNode& node, Second prevTime, Second crntTime, Bool& updated) override
	{
		updated = false;

		const MoveComponent& move = node.getFirstComponentOfType<MoveComponent>();
		if(move.getTimestamp() == node.getGlobalTimestamp())
		{
			GpuParticleEmitterNode& mnode = static_cast<GpuParticleEmitterNode&>(node);
			mnode.onMoveComponentUpdate(move);
		}

		return Error::NONE;
	}
};

GpuParticleEmitterNode::GpuParticleEmitterNode(SceneGraph* scene, CString name)
	: SceneNode(scene, name)
{
}

GpuParticleEmitterNode::~GpuParticleEmitterNode()
{
}

Error GpuParticleEmitterNode::init(const CString& filename)
{
	// Create program
	ANKI_CHECK(getResourceManager().loadResource("shaders/GpuParticlesSimulation.ankiprog", m_prog));
	const ShaderProgramResourceVariant* variant;
	m_prog->getOrCreateVariant(variant);
	m_grProg = variant->getProgram();
	m_workgroupSizeX = variant->getWorkgroupSizes()[0];

	// Load particle props
	ANKI_CHECK(getResourceManager().loadResource(filename, m_emitterRsrc));

	// Create a UBO with the props
	BufferInitInfo buffInit;
	buffInit.m_mapAccess = BufferMapAccessBit::WRITE;
	buffInit.m_usage = BufferUsageBit::UNIFORM_COMPUTE;
	buffInit.m_size = sizeof(GpuParticleEmitterProperties);
	m_propsBuff = getSceneGraph().getGrManager().newBuffer(buffInit);
	GpuParticleEmitterProperties* props =
		static_cast<GpuParticleEmitterProperties*>(m_propsBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));

	const ParticleEmitterProperties& inProps = m_emitterRsrc->getProperties();
	props->m_minGravity = inProps.m_particle.m_minGravity;
	props->m_minMass = inProps.m_particle.m_minMass;
	props->m_maxGravity = inProps.m_particle.m_maxGravity;
	props->m_maxMass = inProps.m_particle.m_maxMass;
	props->m_minForce = inProps.m_particle.m_minForceDirection * inProps.m_particle.m_minForceMagnitude;
	props->m_minLife = F32(inProps.m_particle.m_minLife);
	props->m_maxForce = inProps.m_particle.m_maxForceDirection * inProps.m_particle.m_maxForceMagnitude;
	props->m_maxLife = F32(inProps.m_particle.m_maxLife);
	props->m_minStartingPosition = inProps.m_particle.m_minStartingPosition;
	props->m_maxStartingPosition = inProps.m_particle.m_maxStartingPosition;
	props->m_particleCount = inProps.m_maxNumOfParticles;

	m_propsBuff->unmap();

	m_particleCount = inProps.m_maxNumOfParticles;

	// Create the particle buffer
	buffInit.m_mapAccess = BufferMapAccessBit::WRITE;
	buffInit.m_usage = BufferUsageBit::ALL_STORAGE;
	buffInit.m_size = sizeof(GpuParticle) * inProps.m_maxNumOfParticles;
	m_particlesBuff = getSceneGraph().getGrManager().newBuffer(buffInit);

	GpuParticle* particle = static_cast<GpuParticle*>(m_particlesBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));
	const GpuParticle* end = particle + inProps.m_maxNumOfParticles;
	for(; particle < end; ++particle)
	{
		particle->m_life = -1.0f; // Force GPU to init the particle
	}

	m_particlesBuff->unmap();

	// Create the rand buffer
	buffInit.m_size = sizeof(U32) + MAX_RAND_FACTORS * sizeof(F32);
	m_randFactorsBuff = getSceneGraph().getGrManager().newBuffer(buffInit);

	F32* randFactors = static_cast<F32*>(m_randFactorsBuff->map(0, MAX_PTR_SIZE, BufferMapAccessBit::WRITE));

	*reinterpret_cast<U32*>(randFactors) = MAX_RAND_FACTORS;
	++randFactors;

	const F32* randFactorsEnd = randFactors + MAX_RAND_FACTORS;
	for(; randFactors < randFactorsEnd; ++randFactors)
	{
		*randFactors = getRandomRange(0.0f, 1.0f);
	}

	m_randFactorsBuff->unmap();

	// Create the sampler
	{
		SamplerInitInfo sinit;
		sinit.m_addressing = SamplingAddressing::CLAMP;

		m_nearestAnyClampSampler = getSceneGraph().getGrManager().newSampler(sinit);
	}

	// Find the extend of the particles
	{
		const Vec3 maxForce = inProps.m_particle.m_maxForceDirection * inProps.m_particle.m_maxForceMagnitude;
		const Vec3& maxGravity = inProps.m_particle.m_maxGravity;
		const F32 maxMass = inProps.m_particle.m_maxMass;
		const F32 maxTime = F32(inProps.m_particle.m_maxLife);

		const Vec3 totalForce = maxGravity * maxMass + maxForce;
		const Vec3 accelleration = totalForce / maxMass;
		const Vec3 velocity = accelleration * maxTime;
		m_maxDistanceAParticleCanGo = (velocity * maxTime).getLength();
	}

	// Create the components
	newComponent<MoveComponent>();
	newComponent<MoveFeedbackComponent>();
	newComponent<SpatialComponent>(this, &m_spatialVolume);
	GenericGpuComputeJobComponent* gpuComp = newComponent<GenericGpuComputeJobComponent>();
	gpuComp->setCallback(
		[](GenericGpuComputeJobQueueElementContext& ctx, const void* userData) {
			static_cast<const GpuParticleEmitterNode*>(userData)->simulate(ctx);
		},
		this);
	RenderComponent* rcomp = newComponent<RenderComponent>();
	rcomp->initRaster(
		[](RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData) {
			ANKI_ASSERT(userData.getSize() == 1);
			static_cast<const GpuParticleEmitterNode*>(userData[0])->draw(ctx);
		},
		this,
		0 // No merging
	);
	rcomp->setFlagsFromMaterial(m_emitterRsrc->getMaterial());

	return Error::NONE;
}

void GpuParticleEmitterNode::onMoveComponentUpdate(const MoveComponent& movec)
{
	const Vec4& pos = movec.getWorldTransform().getOrigin();

	// Update the AABB
	m_spatialVolume.setMin((pos - m_maxDistanceAParticleCanGo).xyz());
	m_spatialVolume.setMax((pos + m_maxDistanceAParticleCanGo).xyz());
	SpatialComponent& spatialc = getFirstComponentOfType<SpatialComponent>();
	spatialc.markForUpdate();
	spatialc.setSpatialOrigin(pos);

	// Stash the position
	m_worldPosition = pos.xyz();
	m_worldRotation = movec.getWorldTransform().getRotation();
}

void GpuParticleEmitterNode::simulate(GenericGpuComputeJobQueueElementContext& ctx) const
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	cmdb->bindShaderProgram(m_grProg);

	// Bind resources
	cmdb->bindStorageBuffer(1, 0, m_particlesBuff, 0, MAX_PTR_SIZE);
	cmdb->bindUniformBuffer(1, 1, m_propsBuff, 0, MAX_PTR_SIZE);
	cmdb->bindStorageBuffer(1, 2, m_randFactorsBuff, 0, MAX_PTR_SIZE);
	cmdb->bindSampler(1, 3, m_nearestAnyClampSampler);

	StagingGpuMemoryToken token;
	GpuParticleSimulationState* unis =
		static_cast<GpuParticleSimulationState*>(ctx.m_stagingGpuAllocator->allocateFrame(
			sizeof(GpuParticleSimulationState), StagingGpuMemoryType::UNIFORM, token));

	unis->m_viewProjMat = ctx.m_viewProjectionMatrix;
	unis->m_unprojectionParams = ctx.m_projectionMatrix.extractPerspectiveUnprojectionParams();
	unis->m_randomIndex = rand();
	unis->m_dt = F32(m_dt);
	unis->m_emitterPosition = m_worldPosition;
	unis->m_emitterRotation = m_worldRotation;
	unis->m_invViewRotation = Mat3x4(Vec3(0.0f), ctx.m_cameraTransform.getRotationPart());
	cmdb->bindUniformBuffer(1, 4, token.m_buffer, token.m_offset, token.m_range);

	// Dispatch
	const U32 workgroupCount = (m_particleCount + m_workgroupSizeX - 1) / m_workgroupSizeX;
	cmdb->dispatchCompute(workgroupCount, 1, 1);
}

void GpuParticleEmitterNode::draw(RenderQueueDrawContext& ctx) const
{
	CommandBufferPtr& cmdb = ctx.m_commandBuffer;

	if(!ctx.m_debugDraw)
	{
		// Program
		ShaderProgramPtr prog;
		m_emitterRsrc->getRenderingInfo(ctx.m_key, prog);
		cmdb->bindShaderProgram(prog);

		// Resources
		static const Mat4 identity = Mat4::getIdentity();

		RenderComponent::allocateAndSetupUniforms(m_emitterRsrc->getMaterial(), ctx, ConstWeakArray<Mat4>(&identity, 1),
												  ConstWeakArray<Mat4>(&identity, 1), *ctx.m_stagingGpuAllocator);

		cmdb->bindStorageBuffer(0, 1, m_particlesBuff, 0, MAX_PTR_SIZE);

		StagingGpuMemoryToken token;
		Vec4* extraUniforms = static_cast<Vec4*>(
			ctx.m_stagingGpuAllocator->allocateFrame(sizeof(Vec4), StagingGpuMemoryType::UNIFORM, token));
		*extraUniforms = ctx.m_cameraTransform.getColumn(2);
		cmdb->bindUniformBuffer(0, 2, token.m_buffer, token.m_offset, token.m_range);

		// Draw
		cmdb->setLineWidth(8.0f);
		cmdb->drawArrays(PrimitiveTopology::LINES, m_particleCount * 2);
	}
	else
	{
		// TODO
	}
}

} // end namespace anki
