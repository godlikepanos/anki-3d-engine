#include "anki/script/ScriptManager.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/script/Common.h"

namespace anki {

//==============================================================================
void ScriptManager::init()
{
	ANKI_LOGI("Initializing scripting engine...");

	// Math
	ANKI_SCRIPT_CALL_WRAP(Vec2);
	ANKI_SCRIPT_CALL_WRAP(Vec3);
	ANKI_SCRIPT_CALL_WRAP(Vec4);

	// Renderer
	ANKI_SCRIPT_CALL_WRAP(Dbg);
	ANKI_SCRIPT_CALL_WRAP(MainRenderer);
	ANKI_SCRIPT_CALL_WRAP(MainRendererSingleton);

	ANKI_LOGI("Scripting engine initialized");
}

} // end namespace anki
