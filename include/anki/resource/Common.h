// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_COMMON_H
#define ANKI_RESOURCE_COMMON_H

#include "anki/util/Allocator.h"
#include "anki/util/DArray.h"
#include "anki/util/String.h"

namespace anki {

// Forward
class GrManager;
class ResourceManager;
template<typename Type>
class ResourcePointer;

/// @addtogroup resource
/// @{

// NOTE: Add resources in 3 places
#define ANKI_FORWARD(rsrc_, name_) \
	class rsrc_; \
	using name_ = ResourcePointer<rsrc_>;

ANKI_FORWARD(Animation, AnimationResourcePointer)
ANKI_FORWARD(TextureResource, TextureResourcePointer)
ANKI_FORWARD(ShaderResource, ShaderResourcePointer)
ANKI_FORWARD(Material, MaterialResourcePointer)
ANKI_FORWARD(Mesh, MeshResourcePointer)
ANKI_FORWARD(BucketMesh, BucketMeshResourcePointer)
ANKI_FORWARD(Skeleton, SkeletonResourcePointer)
ANKI_FORWARD(ParticleEmitterResource, ParticleEmitterResourcePointer)
ANKI_FORWARD(Model, ModelResourcePointer)
ANKI_FORWARD(Script, ScriptResourcePointer)
ANKI_FORWARD(DummyRsrc, DummyResourcePointer)

#undef ANKI_FORWARD

template<typename T>
using ResourceAllocator = HeapAllocator<T>;

template<typename T>
using TempResourceAllocator = StackAllocator<T>;
/// @}

} // end namespace anki

#endif
