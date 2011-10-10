#include "ScriptCommon.h"
#include "core/Globals.h"
#include "core/Logger.h"
#include "renderer/MainRenderer.h"
#include "input/Input.h"
#include "scene/Scene.h"
#include "core/App.h"
#include "event/EventManager.h"


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
