// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCRIPT_SCRIPT_MANAGER_H
#define ANKI_SCRIPT_SCRIPT_MANAGER_H

#include "anki/script/LuaBinder.h"

namespace anki {

// Forward
class SceneGraph;

/// @addtogroup script
/// @{

/// The scripting manager
class ScriptManager: public LuaBinder
{
public:
	ScriptManager(HeapAllocator<U8>& alloc, SceneGraph* scene);

	~ScriptManager();

	/// @privatesection
	/// @{
	SceneGraph& _getSceneGraph()
	{
		return *m_scene;
	}
	/// @}

public:
	SceneGraph* m_scene = nullptr;
};

/// @}

} // end namespace anki

#endif
