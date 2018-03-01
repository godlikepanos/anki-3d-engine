// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/resource/ParticleEmitterResource.h>

namespace anki
{

class ParticleEmitterNode;

/// @addtogroup scene
/// @{

/// Particle base
class ParticleBase
{
	friend class ParticleEmitterNode;

public:
	ParticleBase()
	{
	}

	virtual ~ParticleBase()
	{
	}

	Second getTimeOfBirth() const
	{
		return m_timeOfBirth;
	}

	Second& getTimeOfBirth()
	{
		return m_timeOfBirth;
	}

	void setTimeOfBirth(const Second x)
	{
		m_timeOfBirth = x;
	}

	Second getTimeOfDeath() const
	{
		return m_timeOfDeath;
	}

	Second& getTimeOfDeath()
	{
		return m_timeOfDeath;
	}

	void setTimeOfDeath(const Second x)
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
	virtual void revive(const ParticleEmitterNode& pe, const Transform& trf, Second prevUpdateTime, Second crntTime);

	/// Only relevant for non-bullet simulations
	virtual void simulate(const ParticleEmitterNode& pe, Second prevUpdateTime, Second crntTime)
	{
		(void)pe;
		(void)prevUpdateTime;
		(void)crntTime;
	}

	virtual const Vec4& getPosition() const = 0;

protected:
	Second m_timeOfBirth; ///< Keep the time of birth for nice effects
	Second m_timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead
	Second m_size = 1.0;
	Second m_alpha = 1.0;
};

/// Simple particle for simple simulation
class ParticleSimple : public ParticleBase
{
public:
	ParticleSimple()
	{
	}

	void revive(const ParticleEmitterNode& pe, const Transform& trf, Second prevUpdateTime, Second crntTime) override;

	void simulate(const ParticleEmitterNode& pe, Second prevUpdateTime, Second crntTime) override;

	const Vec4& getPosition() const override
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

	void revive(const ParticleEmitterNode& pe,
		F32 prevUpdateTime, F32 crntTime);

private:
	RigidBody* m_body;
};
#endif

/// The particle emitter scene node. This scene node emitts
class ParticleEmitterNode : public SceneNode, private ParticleEmitterProperties
{
	friend class ParticleBase;
	friend class Particle;
	friend class ParticleSimple;
	friend class ParticleEmitterRenderComponent;
	friend class MoveFeedbackComponent;

public:
	ParticleEmitterNode(SceneGraph* scene, CString name);

	~ParticleEmitterNode();

	ANKI_USE_RESULT Error init(const CString& filename);

	/// @name SceneNode virtuals
	/// @{
	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;
	/// @}

private:
	enum class SimulationType : U8
	{
		UNDEFINED,
		SIMPLE,
		PHYSICS_ENGINE
	};

	/// Size of a single vertex.
	static const U VERTEX_SIZE = 5 * sizeof(F32);

	ParticleEmitterResourcePtr m_particleEmitterResource;
	DynamicArray<ParticleBase*> m_particles;
	Second m_timeLeftForNextEmission = 0.0;
	Obb m_obb;

	// Opt: We dont have to make extra calculations if the ParticleEmitterNode's rotation is the identity
	Bool8 m_identityRotation = true;

	U32 m_aliveParticlesCount = 0;

	/// @name Graphics
	/// @{
	U32 m_vertBuffSize = 0;
	void* m_verts = nullptr;
	/// @}

	SimulationType m_simulationType = SimulationType::UNDEFINED;

	void createParticlesSimulation(SceneGraph* scene);
	void createParticlesSimpleSimulation();

	void onMoveComponentUpdate(MoveComponent& move);

	void setupRenderableQueueElement(RenderableQueueElement& el) const
	{
		el.m_callback = drawCallback;
		el.m_mergeKey = 0;
		el.m_userData = this;
	}

	static void drawCallback(RenderQueueDrawContext& ctx, ConstWeakArray<void*> userData);
};
/// @}

} // end namespace anki
