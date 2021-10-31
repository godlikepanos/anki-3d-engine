// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Gr/Enums.h>
#include <AnKi/Shaders/Include/ModelTypes.h>

namespace anki {

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

enum class RayType : U8
{
	SHADOWS,
	GI,
	REFLECTIONS,
	PATH_TRACING,

	COUNT,
	FIRST = 0
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RayType)

enum class RayTypeBit : U8
{
	NONE,
	SHADOWS = 1 << 0,
	GI = 1 << 1,
	REFLECTIONS = 1 << 2,
	PATH_TRACING = 1 << 3,

	ALL = SHADOWS | GI | REFLECTIONS | PATH_TRACING
};
ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(RayTypeBit)
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

#include <AnKi/Resource/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_RESOURCE
#undef ANKI_INSTANSIATE_RESOURCE_DELIMITER

template<typename T>
using ResourceAllocator = HeapAllocator<T>;

template<typename T>
using TempResourceAllocator = StackAllocator<T>;

/// An alias that denotes a ResourceFilesystem path.
using ResourceFilename = CString;
/// @}

} // end namespace anki
