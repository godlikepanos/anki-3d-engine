// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Allocator.h>
#include <AnKi/Util/Array.h>
#include <AnKi/Util/Assert.h>
#include <AnKi/Util/Atomic.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/BitMask.h>
#include <AnKi/Util/DynamicArray.h>
#include <AnKi/Util/WeakArray.h>
#include <AnKi/Util/Enum.h>
#include <AnKi/Util/File.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/Hash.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/Logger.h>
#include <AnKi/Util/MemoryPool.h>
#include <AnKi/Util/Hierarchy.h>
#include <AnKi/Util/Ptr.h>
#include <AnKi/Util/Singleton.h>
#include <AnKi/Util/StdTypes.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/StringList.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/Thread.h>
#include <AnKi/Util/ThreadPool.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/Visitor.h>
#include <AnKi/Util/INotify.h>
#include <AnKi/Util/SparseArray.h>
#include <AnKi/Util/ObjectAllocator.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Util/Serializer.h>
#include <AnKi/Util/Xml.h>
#include <AnKi/Util/F16.h>
#include <AnKi/Util/Function.h>
#include <AnKi/Util/BuddyAllocatorBuilder.h>
#include <AnKi/Util/StackAllocatorBuilder.h>
#include <AnKi/Util/ClassAllocatorBuilder.h>
#include <AnKi/Util/SegregatedListsAllocatorBuilder.h>

/// @defgroup util Utilities (like STL)

/// @defgroup util_containers STL compatible containers
/// @ingroup util

/// @defgroup util_memory Memory management
/// @ingroup util

/// @defgroup util_file Filesystem
/// @ingroup util

/// @defgroup util_time Time
/// @ingroup util

/// @defgroup util_patterns Design patterns
/// @ingroup util

/// @defgroup util_system System
/// @ingroup util

/// @defgroup util_file Filesystem
/// @ingroup util

/// @defgroup util_thread Threading
/// @ingroup util

/// @defgroup util_logging Logging
/// @ingroup util

/// @defgroup util_other Other interfaces
/// @ingroup util

/// @defgroup util_private Private interfaces
/// @ingroup util
