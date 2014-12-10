// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/physics/PhysicsWorld.h"
#include "anki/scene/SceneGraph.h"

namespace anki {

//==============================================================================
// Ugly but there is no other way
static SceneGraph* gScene = nullptr;

static void* newtonAlloc(int size)
{
	return gScene->getAllocator().allocate(size);
}

static void newtonFree(void* const ptr, int size)
{
	gScene->getAllocator().deallocate(ptr, size);
}

//==============================================================================
PhysicsWorld::PhysicsWorld()
{}

//==============================================================================
PhysicsWorld::~PhysicsWorld()
{
	if(m_world)
	{
		NewtonDestroy(m_world);
	}
}

//==============================================================================
Error PhysicsWorld::create(SceneGraph* scene)
{
	Error err = ErrorCode::NONE;

	ANKI_ASSERT(scene);
	m_scene = scene;
	
	// Set allocators
	gScene = scene;
	NewtonSetMemorySystem(newtonAlloc, newtonFree);

	// Initialize world
	m_world = NewtonCreate();
	if(!m_world)
	{
		ANKI_LOGE("NewtonCreate() failed");
		return ErrorCode::FUNCTION_FAILED;
	}

	return err;
}

} // end namespace anki
