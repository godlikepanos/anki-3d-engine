// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/physics/PhysicsWorld.h>
#include <anki/physics/PhysicsPlayerController.h>
#include <anki/physics/PhysicsCollisionShape.h>
#include <anki/physics/PhysicsBody.h>

namespace anki
{

// Ugly but there is no other way
static HeapAllocator<U8>* gAlloc = nullptr;

static void* newtonAlloc(int size)
{
	return gAlloc->allocate(size + 16);
}

static void newtonFree(void* const ptr, int size)
{
	gAlloc->deallocate(ptr, size + 16);
}

PhysicsWorld::PhysicsWorld()
{
}

PhysicsWorld::~PhysicsWorld()
{
	cleanupMarkedForDeletion();

	if(m_sceneBody)
	{
		NewtonDestroyBody(m_sceneBody);
		m_sceneBody = nullptr;
	}

	if(m_sceneCollision)
	{
		NewtonDestroyCollision(m_sceneCollision);
		m_sceneCollision = nullptr;
	}

	if(m_world)
	{
		NewtonDestroy(m_world);
		m_world = nullptr;
	}

	gAlloc = nullptr;
}

Error PhysicsWorld::create(AllocAlignedCallback allocCb, void* allocCbData)
{
	Error err = ErrorCode::NONE;

	m_alloc = HeapAllocator<U8>(allocCb, allocCbData);

	// Set allocators
	gAlloc = &m_alloc;
	NewtonSetMemorySystem(newtonAlloc, newtonFree);

	// Initialize world
	m_world = NewtonCreate();
	if(!m_world)
	{
		ANKI_PHYS_LOGE("NewtonCreate() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	// Set the simplified solver mode (faster but less accurate)
	NewtonSetSolverModel(m_world, 1);

	// Create the character controller manager. Newton needs it's own allocators
	m_playerManager = new CharacterControllerManager(this);

	// Create scene collision
	m_sceneCollision = NewtonCreateSceneCollision(m_world, 0);
	Mat4 trf = Mat4::getIdentity();
	m_sceneBody = NewtonCreateDynamicBody(m_world, m_sceneCollision, &trf[0]);
	NewtonBodySetMaterialGroupID(m_sceneBody, NewtonMaterialGetDefaultGroupID(m_world));

	NewtonDestroyCollision(m_sceneCollision); // destroy old scene
	m_sceneCollision = NewtonBodyGetCollision(m_sceneBody);

	// Set callbacks
	NewtonMaterialSetCollisionCallback(m_world,
		NewtonMaterialGetDefaultGroupID(m_world),
		NewtonMaterialGetDefaultGroupID(m_world),
		onAabbOverlapCallback,
		onContactCallback);

	return err;
}

Error PhysicsWorld::updateAsync(F32 dt)
{
	m_dt = dt;

	// Do cleanup of marked for deletion
	cleanupMarkedForDeletion();

	// Update
	NewtonUpdateAsync(m_world, dt);

	return ErrorCode::NONE;
}

void PhysicsWorld::waitUpdate()
{
	NewtonWaitForUpdateToFinish(m_world);
}

void PhysicsWorld::cleanupMarkedForDeletion()
{
	LockGuard<Mutex> lock(m_mtx);

	while(!m_forDeletion.isEmpty())
	{
		auto it = m_forDeletion.getBegin();
		PhysicsObject* obj = *it;

		// Remove from objects marked for deletion
		m_forDeletion.erase(m_alloc, it);

		// Finaly, delete it
		m_alloc.deleteInstance(obj);
	}
}

void PhysicsWorld::registerObject(PhysicsObject* ptr)
{
	// TODO Remove
}

void PhysicsWorld::onContactCallback(const NewtonJoint* contactJoint, F32 timestep, int threadIndex)
{
	const NewtonBody* body0 = NewtonJointGetBody0(contactJoint);
	const NewtonBody* body1 = NewtonJointGetBody1(contactJoint);

	F32 friction0 = 0.01;
	F32 elasticity0 = 0.001;
	F32 friction1 = friction0;
	F32 elasticity1 = elasticity0;

	void* userData = NewtonBodyGetUserData(body0);
	if(userData)
	{
		friction0 = static_cast<PhysicsBody*>(userData)->getFriction();
		elasticity0 = static_cast<PhysicsBody*>(userData)->getElasticity();
	}

	userData = NewtonBodyGetUserData(body1);
	if(userData)
	{
		friction1 = static_cast<PhysicsBody*>(userData)->getFriction();
		elasticity1 = static_cast<PhysicsBody*>(userData)->getElasticity();
	}

	F32 friction = friction0 + friction1;
	F32 elasticity = elasticity0 + elasticity1;

	void* contact = NewtonContactJointGetFirstContact(contactJoint);
	while(contact)
	{
		NewtonMaterial* material = NewtonContactGetMaterial(contact);

		NewtonMaterialSetContactFrictionCoef(material, friction + 0.1, friction, 0);
		NewtonMaterialSetContactFrictionCoef(material, friction + 0.1, friction, 1);

		NewtonMaterialSetContactElasticity(material, elasticity);

		contact = NewtonContactJointGetNextContact(contactJoint, contact);
	}
}

} // end namespace anki
