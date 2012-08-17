#include "anki/script/ScriptManager.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/script/Common.h"

namespace anki {

//==============================================================================
void ScriptManager::init()
{
	ANKI_LOGI("Initializing scripting engine...");

	ANKI_SCRIPT_CALL_WRAP(Vec2);

	ANKI_LOGI("Scripting engine initialized");
}

//==============================================================================
void ScriptManager::execScript(const char* script, const char* scriptName)
{
	// XXX
}

} // end namespace
