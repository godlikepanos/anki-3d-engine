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
/// XXX Remove SceneNode
class ParticleBase: public SceneNode, public MoveComponent
{
	friend class ParticleEmitter;

public:
	enum ParticleType
	{
		PT_SIMPLE,
		PT_PHYSICS
	};

	ParticleBase(
		const char* name, SceneGraph* scene, // SceneNode
		ParticleType type); // Self

	virtual ~ParticleBase();

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

protected:
	F32 timeOfBirth; ///< Keep the time of birth for nice effects
	F32 timeOfDeath = -1.0; ///< Time of death. If < 0.0 then dead. In seconds
	F32 size = 1.0;
	F32 alpha = 1.0;

private:
	ParticleType type;
};

/// Simple particle for simple simulation
class ParticleSimple: public ParticleBase
{
public:
	ParticleSimple(
		const char* name, SceneGraph* scene); // SceneNode

	void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime);

	void simulate(const ParticleEmitter& pe, F32 prevUpdateTime, F32 crntTime);

private:
	/// The velocity
	Vec3 velocity = Vec3(0.0);
	Vec3 acceleration = Vec3(0.0);
};

/// Particle for bullet simulations
class Particle: public ParticleBase, public RigidBody
{
public:
	Particle(
		const char* name, SceneGraph* scene, // SceneNode
		// RigidBody
		PhysWorld* masterContainer, const RigidBody::Initializer& init); 

	~Particle();

	void kill()
	{
		ParticleBase::kill();
		setActivationState(DISABLE_SIMULATION);
	}

	void revive(const ParticleEmitter& pe,
		F32 prevUpdateTime, F32 crntTime);
};

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

	/// Override SceneNode::frameUpdate()
	void frameUpdate(F32 prevUpdateTime, F32 crntTime, I frame);
	/// @}

	/// @name RenderComponent virtuals
	/// @{

	/// Implements RenderComponent::getRenderingData
	void getRenderingData(
		const PassLodKey& key, 
		const Vao*& vao, const ShaderProgram*& prog,
		const U32* subMeshIndicesArray, U subMeshIndicesCount,
		Array<U32, ANKI_MAX_MULTIDRAW_PRIMITIVES>& indicesCountArray,
		Array<const void*, ANKI_MAX_MULTIDRAW_PRIMITIVES>& indicesOffsetArray, 
		U32& drawcallCount) const;

	/// Implements  RenderComponent::getMaterial
	const Material& getMaterial();

	/// Overrides RenderComponent::getRenderWorldTransforms
	const Transform* getRenderWorldTransforms()
	{
		return &(*instancingTransformations)[0];
	}

	/// Overrides RenderComponent::getRenderInstancesCount
	U32 getRenderInstancesCount()
	{
		return instancesCount;
	}
	/// @}

	/// @name MoveComponent virtuals
	/// @{

	/// Overrides MoveComponent::moveUpdate. Calculates an optimization
	void moveUpdate();
	/// @}

private:
	ParticleEmitterResourcePointer particleEmitterResource;
	btCollisionShape* collShape = nullptr;
	SceneVector<ParticleBase*> particles;
	F32 timeLeftForNextEmission;
	/// The resource
	Aabb aabb;

	// Opt: We dont have to make extra calculations if the ParticleEmitter's
	// rotation is the identity
	Bool identityRotation = true;

	U32 instancesCount; ///< AKA alive count

	/// The transformations. Calculated on frameUpdate() and consumed on
	/// rendering.
	SceneFrameVector<Transform>* instancingTransformations;

	RenderComponentVariable* alphaRenderComponentVar = nullptr;

	void createParticlesSimulation(SceneGraph* scene);
	void createParticlesSimpleSimulation(SceneGraph* scene);
};

} // end namespace anki

#endif
