// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
class TransferGpuAllocatorHandle;

/// @addtogroup resource
/// @{

#define ANKI_RESOURCE_LOGI(...) ANKI_LOG("RSRC", NORMAL, __VA_ARGS__)
#define ANKI_RESOURCE_LOGE(...) ANKI_LOG("RSRC", ERROR, __VA_ARGS__)
#define ANKI_RESOURCE_LOGW(...) ANKI_LOG("RSRC", WARNING, __VA_ARGS__)
#define ANKI_RESOURCE_LOGF(...) ANKI_LOG("RSRC", FATAL, __VA_ARGS__)

/// @name Constants
/// @{
const U MAX_LOD_COUNT = 3;
const U MAX_INSTANCES = 64;
const U MAX_SUB_DRAWCALLS = 64; ///< @warning If changed don't forget to change MAX_INSTANCE_GROUPS
const U MAX_INSTANCE_GROUPS = 7; ///< It's log2(MAX_INSTANCES) + 1

/// Standard attribute locations. Should be the same as in Common.glsl.
enum class VertexAttributeLocation : U8
{
	POSITION,
	UV,
	UV2,
	NORMAL,
	TANGENT,
	COLOR,
	BONE_WEIGHTS,
	BONE_INDICES,

	COUNT,
	FIRST = POSITION,
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(VertexAttributeLocation, inline)
/// @}

/// Deleter for ResourcePtr.
template<typename T>
class ResourcePtrDeleter
{
public:
	void operator()(T* ptr);
};

/// Smart pointer for resources.
template<typename T>
using ResourcePtr = IntrusivePtr<T, ResourcePtrDeleter<T>>;

// NOTE: Add resources in 3 places
#define ANKI_INSTANTIATE_RESOURCE(rsrc_, name_) \
	class rsrc_; \
	using name_ = ResourcePtr<rsrc_>;

#define ANKI_INSTANSIATE_RESOURCE_DELIMITER()

#include <anki/resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

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
