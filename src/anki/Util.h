// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Allocator.h>
#include <anki/util/Array.h>
#include <anki/util/Assert.h>
#include <anki/util/Atomic.h>
#include <anki/util/BitSet.h>
#include <anki/util/BitMask.h>
#include <anki/util/DynamicArray.h>
#include <anki/util/WeakArray.h>
#include <anki/util/Enum.h>
#include <anki/util/File.h>
#include <anki/util/Filesystem.h>
#include <anki/util/Functions.h>
#include <anki/util/Hash.h>
#include <anki/util/HighRezTimer.h>
#include <anki/util/List.h>
#include <anki/util/Logger.h>
#include <anki/util/Memory.h>
#include <anki/util/NonCopyable.h>
#include <anki/util/Hierarchy.h>
#include <anki/util/Ptr.h>
#include <anki/util/Singleton.h>
#include <anki/util/StdTypes.h>
#include <anki/util/String.h>
#include <anki/util/StringList.h>
#include <anki/util/System.h>
#include <anki/util/Thread.h>
#include <anki/util/ThreadPool.h>
#include <anki/util/Visitor.h>
#include <anki/util/INotify.h>
#include <anki/util/SparseArray.h>
#include <anki/util/ObjectAllocator.h>
#include <anki/util/Tracer.h>

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
