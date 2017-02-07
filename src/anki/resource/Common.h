// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/String.h>
#include <anki/util/Ptr.h>
#include <anki/gr/Enums.h>

namespace anki
{

// Forward
class GrManager;
class ResourceManager;
class ResourceFilesystem;
template<typename Type>
class ResourcePointer;

/// @addtogroup resource
/// @{

#define ANKI_RESOURCE_LOGI(...) ANKI_LOG("RSRC", NORMAL, __VA_ARGS__)
#define ANKI_RESOURCE_LOGE(...) ANKI_LOG("RSRC", ERROR, __VA_ARGS__)
#define ANKI_RESOURCE_LOGW(...) ANKI_LOG("RSRC", WARNING, __VA_ARGS__)
#define ANKI_RESOURCE_LOGF(...) ANKI_LOG("RSRC", FATAL, __VA_ARGS__)

/// @name Constants
/// @{
const U MAX_LODS = 3;
const U MAX_INSTANCES = 64;
const U MAX_SUB_DRAWCALLS = 64;

/// The number of instance groups. Eg First group is 1 instance, 2nd group 2 instances, 3rd is 4 instances. The
/// expression is: @code log2(MAX_INSTANCES) + 1 @endcode but since Clang doesn't like log2 in constant expressions use
/// an alternative way.
const U MAX_INSTANCE_GROUPS = __builtin_popcount(MAX_INSTANCES - 1) + 1;
/// @}

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
#define ANKI_FORWARD(rsrc_, name_)                                                                                     \
	class rsrc_;                                                                                                       \
	using name_ = ResourcePtr<rsrc_>;

ANKI_FORWARD(Animation, AnimationResourcePtr)
ANKI_FORWARD(TextureResource, TextureResourcePtr)
ANKI_FORWARD(ShaderResource, ShaderResourcePtr)
ANKI_FORWARD(Material, MaterialResourcePtr)
ANKI_FORWARD(Mesh, MeshResourcePtr)
ANKI_FORWARD(Skeleton, SkeletonResourcePtr)
ANKI_FORWARD(ParticleEmitterResource, ParticleEmitterResourcePtr)
ANKI_FORWARD(Model, ModelResourcePtr)
ANKI_FORWARD(Script, ScriptResourcePtr)
ANKI_FORWARD(DummyRsrc, DummyResourcePtr)
ANKI_FORWARD(CollisionResource, CollisionResourcePtr)
ANKI_FORWARD(GenericResource, GenericResourcePtr)
ANKI_FORWARD(TextureAtlas, TextureAtlasResourcePtr)

#undef ANKI_FORWARD

template<typename T>
using ResourceAllocator = HeapAllocator<T>;

template<typename T>
using TempResourceAllocator = StackAllocator<T>;

/// An alias that denotes a ResourceFilesystem path.
using ResourceFilename = CString;

/// Given a shader type return the appropriate file extension.
ANKI_USE_RESULT const CString& shaderTypeToFileExtension(ShaderType type);

/// Given a filename return the shader type.
ANKI_USE_RESULT Error fileExtensionToShaderType(const CString& filename, ShaderType& type);
/// @}

} // end namespace anki
