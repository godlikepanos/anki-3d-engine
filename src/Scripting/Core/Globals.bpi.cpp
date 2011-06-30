#include "ScriptingCommon.h"
#include "Core/Globals.h"
#include "Core/Logger.h"
#include "MainRenderer.h"
#include "Input.h"
#include "Scene.h"
#include "App.h"
#include "Events/Manager.h"


WRAP_SINGLETON(LoggerSingleton)
WRAP_SINGLETON(MainRendererSingleton)
WRAP_SINGLETON(InputSingleton)
WRAP_SINGLETON(SceneSingleton)
WRAP_SINGLETON(AppSingleton)

typedef Event::ManagerSingleton EventManagerSingleton;
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
