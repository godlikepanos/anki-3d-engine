// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCRIPT_SCRIPT_MANAGER_H
#define ANKI_SCRIPT_SCRIPT_MANAGER_H

#include "anki/script/LuaBinder.h"
#include "anki/util/Singleton.h"

namespace anki {

/// The scripting manager
class ScriptManager: public LuaBinder
{
public:
	ScriptManager(HeapAllocator<U8>& alloc);

	~ScriptManager();
};

typedef SingletonInit<ScriptManager> ScriptManagerSingleton;

} // end namespace anki

#endif
