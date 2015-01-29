// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_PARTICLE_EMITTER_H
#define ANKI_SCENE_PARTICLE_EMITTER_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/RenderComponent.h"
#include "anki/resource/ParticleEmitterResource.h"

namespace anki {

class ParticleEmitter;

/// @addtogroup scene
/// @{

/// Particle base
class ParticleBase
{
	friend class ParticleEmitter;

public:
	ParticleBase()
	{}

	virtual ~ParticleBase()
	{}

	F32 getTimeOfBirth() const
	{
		return m_timeOfBirth;
	}

	F32& getTimeOfBirth()
	{
		return m_timeOfBirth;
	}

	void setTimeOfBirth(const F32 x)
	{
		m_timeOfBirth = x;
	}

	F32 getTimeOfDeath() const
	{
		return m_timeOfDeath;
	}

	F32& getTimeOfDeath()
	{
		return m_timeOfDeath;
	}

	void setTimeOfDeath(const F32 x)
	{
		m_timeOfDeath = x;
	}

	Bool isDead() const
	{
		return m_timeOfDeath < 0.0;
	}

	/// Kill the particle
	virtual void kill()
	{
		ANKI_ASSERT(m_timeOfDeath > 0.0);
		m_timeOfDeath = -1.0;
	}

	/// Revive the particle
	virtual void revive(const ParticleEmitter& pe, const Transform& trf,
		F32 prevUpdateTime, F32 crntTime);

	/// Only relevant for non-bullet simulations
	virtual void simulate(const ParticleEmitter& pe, 
		F32 prevUpdateTime, F32 crntTime)
	{
		(void)pe;
		(void)prevUpdateTime;
		(void)crntTime;
	}

	virtual const Vec4& getPosition() const = 0;

protected:
	F32 m_timeOfBirth; ///< Keep the time of birth for nice effects
	F32 m_timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead
	F32 m_size = 1.0;
	F32 m_alpha = 1.0;
};

/// Simple particle for simple simulation
class ParticleSimple: public ParticleBase
{
public:
	ParticleSimple()
	{}

	void revive(const ParticleEmitter& pe, const Transform& trf,
		F32 prevUpdateTime, F32 crntTime) override;

	void simulate(const ParticleEmitter& pe, F32 prevUpdateTime, 
		F32 crntTime) override;

	const Vec4& getPosition() const
	{
		return m_position;
	}

private:
	/// The velocity
	Vec4 m_velocity = Vec4(0.0);
	Vec4 m_acceleration = Vec4(0.0);
	Vec4 m_position;
};

#if 0
/// Particle for bullet simulations
class Particle: public ParticleBase
{
public:
	Particle(
		const char* name, SceneGraph* scene, // SceneNode
		// RigidBody
		PhysicsWorld* masterContainer, const RigidBody::Initializer& init); 

	~Particle();

	void kill()
	{
		ParticleBase::kill();
		body->setActivationState(DISABLE_SIMULATION);
	}

	void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime);

private:
	RigidBody* m_body;
};
#endif

/// The particle emitter scene node. This scene node emitts
class ParticleEmitter: public SceneNode, private ParticleEmitterProperties
{
	friend class ParticleBase;
	friend class Particle;
	friend class ParticleSimple;
	friend class ParticleEmitterRenderComponent;
	friend class MoveFeedbackComponent;

public:
	ParticleEmitter(SceneGraph* scene);

	~ParticleEmitter();

	ANKI_USE_RESULT Error create(
		const CString& name, const CString& filename);

	/// @name SceneNode virtuals
	/// @{
	ANKI_USE_RESULT Error frameUpdate(
		F32 prevUpdateTime, F32 crntTime) override;
	/// @}

private:
	enum class SimulationType: U8
	{
		UNDEFINED,
		SIMPLE,
		PHYSICS_ENGINE
	};

	ParticleEmitterResourcePointer m_particleEmitterResource;
	SceneDArray<ParticleBase*> m_particles;
	F32 m_timeLeftForNextEmission = 0.0;
	Obb m_obb;
	SceneDArray<Transform> m_transforms; ///< InstanceTransforms
	Timestamp m_transformsTimestamp = 0;

	// Opt: We dont have to make extra calculations if the ParticleEmitter's
	// rotation is the identity
	Bool8 m_identityRotation = true;

	U32 m_aliveParticlesCount = 0;

	GlBufferHandle m_vertBuff; ///< Hold the vertex data
	U8* m_vertBuffMapping = nullptr; 

	SimulationType m_simulationType = SimulationType::UNDEFINED;

	ANKI_USE_RESULT Error createParticlesSimulation(SceneGraph* scene);
	ANKI_USE_RESULT Error createParticlesSimpleSimulation();

	ANKI_USE_RESULT Error doInstancingCalcs();

	ANKI_USE_RESULT Error buildRendering(RenderingBuildData& data);
	void getRenderWorldTransform(U index, Transform& trf);

	void onMoveComponentUpdate(MoveComponent& move);
};
/// @}

} // end namespace anki

#endif
