#ifndef ANKI_SCRIPT_SCRIPT_MANAGER_H
#define ANKI_SCRIPT_SCRIPT_MANAGER_H

#include "anki/script/LuaBinder.h"
#include "anki/util/Singleton.h"

namespace anki {

/// The scripting manager
class ScriptManager: public LuaBinder
{
public:
	ScriptManager()
	{
		init();
	}

private:
	void init();
};

typedef Singleton<ScriptManager> ScriptManagerSingleton;

} // end namespace anki

#endif
