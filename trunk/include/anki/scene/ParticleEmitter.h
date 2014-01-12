#ifndef ANKI_SCENE_PARTICLE_EMITTER_H
#define ANKI_SCENE_PARTICLE_EMITTER_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/MoveComponent.h"
#include "anki/scene/SpatialComponent.h"
#include "anki/scene/RenderComponent.h"
#include "anki/physics/RigidBody.h"
#include "anki/resource/ParticleEmitterResource.h"

namespace anki {

class ParticleEmitter;

/// Particle base
class ParticleBase
{
	friend class ParticleEmitter;

public:
	ParticleBase()
	{}

	virtual ~ParticleBase()
	{}

	/// @name Accessors
	/// @{
	F32 getTimeOfBirth() const
	{
		return timeOfBirth;
	}
	F32& getTimeOfBirth()
	{
		return timeOfBirth;
	}
	void setTimeOfBirth(const F32 x)
	{
		timeOfBirth = x;
	}

	F32 getTimeOfDeath() const
	{
		return timeOfDeath;
	}
	F32& getTimeOfDeath()
	{
		return timeOfDeath;
	}
	void setTimeOfDeath(const F32 x)
	{
		timeOfDeath = x;
	}
	/// @}

	Bool isDead() const
	{
		return timeOfDeath < 0.0;
	}

	/// Kill the particle
	virtual void kill()
	{
		ANKI_ASSERT(timeOfDeath > 0.0);
		timeOfDeath = -1.0;
	}

	/// Revive the particle
	virtual void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime);

	/// Only relevant for non-bullet simulations
	virtual void simulate(const ParticleEmitter& pe, 
		F32 prevUpdateTime, F32 crntTime)
	{
		(void)pe;
		(void)prevUpdateTime;
		(void)crntTime;
	}

	virtual const Vec3& getPosition() const = 0;

protected:
	F32 timeOfBirth; ///< Keep the time of birth for nice effects
	F32 timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead. In seconds
	F32 size = 1.0;
	F32 alpha = 1.0;

private:
	U8 type;
};

/// Simple particle for simple simulation
class ParticleSimple: public ParticleBase
{
public:
	ParticleSimple()
	{}

	void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime) override;

	void simulate(const ParticleEmitter& pe, F32 prevUpdateTime, 
		F32 crntTime) override;

	const Vec3& getPosition() const
	{
		return position;
	}

private:
	/// The velocity
	Vec3 velocity = Vec3(0.0);
	Vec3 acceleration = Vec3(0.0);
	Vec3 position;
};

/// Particle for bullet simulations
#if 0
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
	RigidBody* body;
};
#endif

/// The particle emitter scene node. This scene node emitts
class ParticleEmitter: public SceneNode, public SpatialComponent, 
	public MoveComponent, public RenderComponent, 
	private ParticleEmitterProperties
{
	friend class ParticleBase;
	friend class Particle;
	friend class ParticleSimple;

public:
	ParticleEmitter(
		const char* name, SceneGraph* scene, // SceneNode
		const char* filename); // Self

	~ParticleEmitter();

	/// @name SceneNode virtuals
	/// @{

	void frameUpdate(F32 prevUpdateTime, F32 crntTime, 
		SceneNode::UpdateType uptype) override;

	void componentUpdated(SceneComponent& comp, 
		SceneComponent::UpdateType) override;
	/// @}

	/// @name SpatialComponent virtuals
	/// @{
	const CollisionShape& getSpatialCollisionShape()
	{
		return obb;
	}

	Vec3 getSpatialOrigin()
	{
		return obb.getCenter();
	}
	/// @}

	/// @name RenderComponent virtuals
	/// @{

	void getRenderingData(
		const PassLodKey& key, 
		const U8* subMeshIndicesArray, U subMeshIndicesCount,
		const Vao*& vao, const ShaderProgram*& prog,
		Drawcall& drawcall);

	const Material& getMaterial();

	void getRenderWorldTransform(U index, Transform& trf) override;

	Bool getHasWorldTransforms() override;
	/// @}

private:
	enum SimulationType
	{
		UNDEFINED_SIMULATION,
		SIMPLE_SIMULATION,
		PHYSICS_ENGINE_SIMULATION
	};

	ParticleEmitterResourcePointer particleEmitterResource;
	btCollisionShape* collShape = nullptr;
	SceneVector<ParticleBase*> particles;
	F32 timeLeftForNextEmission;
	Obb obb;
	SceneVector<Transform> transforms; ///< InstanceTransforms
	Timestamp transformsTimestamp = 0;

	// Opt: We dont have to make extra calculations if the ParticleEmitter's
	// rotation is the identity
	Bool identityRotation = true;

	U32 aliveParticlesCount = 0;
	U32 aliveParticlesCountDraw = 0;

	Vao vao; ///< Hold the VBO
	GlBuffer vbo; ///< Hold the vertex data
	SceneVector<F32> clientBuffer;

	U8 simulationType = UNDEFINED_SIMULATION;

	void createParticlesSimulation(SceneGraph* scene);
	void createParticlesSimpleSimulation(SceneGraph* scene);

	void doInstancingCalcs();
};

} // end namespace anki

#endif
