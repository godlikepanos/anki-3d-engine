#include "ScriptingCommon.h"
#include "Core/Globals.h"
#include "Core/Logger.h"
#include "Renderer/MainRenderer.h"
#include "Input/Input.h"
#include "Scene/Scene.h"
#include "Core/App.h"
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
