#include "anki/script/ScriptCommon.h"
#include "anki/core/Globals.h"
#include "anki/core/Logger.h"
#include "anki/renderer/MainRenderer.h"
#include "anki/input/Input.h"
#include "anki/scene/Scene.h"
#include "anki/core/App.h"
#include "anki/event/EventManager.h"


WRAP_SINGLETON(LoggerSingleton)
WRAP_SINGLETON(MainRendererSingleton)
WRAP_SINGLETON(InputSingleton)
WRAP_SINGLETON(SceneSingleton)
WRAP_SINGLETON(AppSingleton)
WRAP_SINGLETON(EventManagerSingleton)


void boostPythonWrapAllGlobals()
{
	CALL_WRAP(LoggerSingleton);
	CALL_WRAP(MainRendererSingleton);
	CALL_WRAP(InputSingleton);
	CALL_WRAP(SceneSingleton);
	CALL_WRAP(AppSingleton);
	CALL_WRAP(EventManagerSingleton);
}
