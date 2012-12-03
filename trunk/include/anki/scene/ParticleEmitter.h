#ifndef ANKI_SCENE_PARTICLE_EMITTER_H
#define ANKI_SCENE_PARTICLE_EMITTER_H

#include "anki/scene/SceneNode.h"
#include "anki/scene/Movable.h"
#include "anki/scene/Spatial.h"
#include "anki/scene/Renderable.h"
#include "anki/physics/RigidBody.h"
#include "anki/resource/ParticleEmitterResource.h"

namespace anki {

/// Particle scene node
class Particle: public SceneNode, public Movable, public RigidBody
{
public:
	Particle(
		F32 timeOfDeath,
		// SceneNode
		const char* name, Scene* scene, 
		// Movable
		U32 movableFlags, Movable* movParent,
		// RigidBody
		PhysWorld* masterContainer, const RigidBody::Initializer& init); 

	~Particle();

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Override SceneNode::getRigidBody()
	RigidBody* getRigidBody()
	{
		return this;
	}
	/// @}

	F32 getTimeOfDeath() const
	{
		return timeOfDeath;
	}
	F32& getTimeOfDeath()
	{
		return timeOfDeath;
	}
	void setTimeOfDeath(F32 x)
	{
		timeOfDeath = x;
	}

	Bool isDead() const
	{
		return timeOfDeath < 0.0;
	}

private:
	F32 timeOfDeath; ///< Life of death. If < 0.0 then dead. In seconds
};

/// The particle emitter scene node. This scene node emitts
class ParticleEmitter: public SceneNode, public Spatial, public Movable,
	public Renderable, private ParticleEmitterProperties
{
public:
	ParticleEmitter(const char* filename,
		const char* name, Scene* scene, // Scene
		U32 movableFlags, Movable* movParent); // Movable
	~ParticleEmitter();

	/// @name SceneNode virtuals
	/// @{

	/// Override SceneNode::getMovable()
	Movable* getMovable()
	{
		return this;
	}

	/// Override SceneNode::getSpatial()
	Spatial* getSpatial()
	{
		return this;
	}

	/// Override SceneNode::getRenderable()
	Renderable* getRenderable()
	{
		return this;
	}

	/// Override SceneNode::frameUpdate()
	void frameUpdate(F32 prevUpdateTime, F32 crntTime, I frame);
	/// @}

	/// @name Renderable virtuals
	/// @{

	/// Implements Renderable::getModelPatchBase
	const ModelPatchBase& getRenderableModelPatchBase() const;

	/// Implements  Renderable::getMaterial
	const Material& getRenderableMaterial() const;

	/// Overrides Renderable::getRenderableInstancingTranslations
	virtual const Vec3* getRenderableInstancingTranslations() const
	{
		return &instancintPositions[0];
	}

	virtual U32 getRenderableInstancesCount() const
	{
		return instancesCount;
	}
	/// @}

	/// @name Movable virtuals
	/// @{

	/// Overrides Movable::movableUpdate. Calculates an optimization
	void movableUpdate();
	/// @}

private:
	ParticleEmitterResourcePointer particleEmitterResource;
	std::unique_ptr<btCollisionShape> collShape;
	PtrVector<Particle> particles;
	F32 timeLeftForNextEmission;
	/// The resource
	Aabb aabb;

	// Opt: We dont have to make extra calculations if the ParticleEmitter's
	// rotation is the identity
	Bool identityRotation = true;

	U32 instancesCount; ///< AKA alive

	Vector<Vec3> instancintPositions;

	void init(const char* filename, Scene* scene);

	static F32 getRandom(F32 initial, F32 deviation);
	static Vec3 getRandom(const Vec3& initial, const Vec3& deviation);
};

} // end namespace anki

#endif
