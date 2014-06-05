// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_THREADPOOL_H
#define ANKI_CORE_THREADPOOL_H

#include "anki/util/Thread.h"
#include "anki/util/Singleton.h"

namespace anki {

/// Singleton
typedef Singleton<Threadpool> ThreadpoolSingleton;

} // end namespace anki

#endif
