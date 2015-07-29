// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/util/Allocator.h"
#include "anki/util/DArray.h"
#include "anki/util/String.h"
#include "anki/util/Ptr.h"

namespace anki {

// Forward
class GrManager;
class ResourceManager;
class ResourceFilesystem;
template<typename Type>
class ResourcePointer;

/// @addtogroup resource
/// @{

/// Deleter for ResourcePtr.
template<typename T>
class ResourcePtrDeleter
{
public:
	void operator()(T* ptr)
	{
		ptr->getManager().unregisterResource(ptr);
		auto alloc = ptr->getAllocator();
		alloc.deleteInstance(ptr);
	}
};

/// Smart pointer for resources.
template<typename T>
using ResourcePtr = IntrusivePtr<T, ResourcePtrDeleter<T>>;

// NOTE: Add resources in 3 places
#define ANKI_FORWARD(rsrc_, name_) \
	class rsrc_; \
	using name_ = ResourcePtr<rsrc_>;

ANKI_FORWARD(Animation, AnimationResourcePtr)
ANKI_FORWARD(TextureResource, TextureResourcePtr)
ANKI_FORWARD(ShaderResource, ShaderResourcePtr)
ANKI_FORWARD(Material, MaterialResourcePtr)
ANKI_FORWARD(Mesh, MeshResourcePtr)
ANKI_FORWARD(BucketMesh, BucketMeshResourcePtr)
ANKI_FORWARD(Skeleton, SkeletonResourcePtr)
ANKI_FORWARD(ParticleEmitterResource, ParticleEmitterResourcePtr)
ANKI_FORWARD(Model, ModelResourcePtr)
ANKI_FORWARD(Script, ScriptResourcePtr)
ANKI_FORWARD(DummyRsrc, DummyResourcePtr)
ANKI_FORWARD(CollisionResource, CollisionResourcePtr)
ANKI_FORWARD(GenericResource, GenericResourcePtr)

#undef ANKI_FORWARD

template<typename T>
using ResourceAllocator = HeapAllocator<T>;

template<typename T>
using TempResourceAllocator = StackAllocator<T>;

/// An alias that denotes a ResourceFilesystem path.
using ResourceFilename = CString;
/// @}

} // end namespace anki

