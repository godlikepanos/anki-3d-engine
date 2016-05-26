// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

namespace anki
{

//==============================================================================
GrManager::GrManager()
{
}

//==============================================================================
GrManager::~GrManager()
{
}

//==============================================================================
Error GrManager::init(GrManagerInitInfo& init)
{
}

//==============================================================================
void GrManager::swapBuffers()
{
}

//==============================================================================
void GrManager::finish()
{
}

//==============================================================================
void* GrManager::allocateFrameTransientMemory(PtrSize size,
	BufferUsage usage,
	TransientMemoryToken& token,
	Error* err)
{
}
	
} // end namespace anki