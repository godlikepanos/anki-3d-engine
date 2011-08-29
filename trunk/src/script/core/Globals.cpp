#include "Common.h"
#include "core/Globals.h"
#include "core/Logger.h"
#include "r/MainRenderer.h"
#include "i/Input.h"
#include "scene/Scene.h"
#include "core/App.h"
#include "event/Manager.h"


WRAP_SINGLETON(LoggerSingleton)

typedef r::MainRendererSingleton r_MainRendererSingleton;
WRAP_SINGLETON(r_MainRendererSingleton)

WRAP_SINGLETON(InputSingleton)
WRAP_SINGLETON(SceneSingleton)
WRAP_SINGLETON(AppSingleton)

typedef event::ManagerSingleton event_ManagerSingleton;
WRAP_SINGLETON(event_ManagerSingleton)


void boostPythonWrapAllGlobals()
{
	CALL_WRAP(LoggerSingleton);
	CALL_WRAP(r_MainRendererSingleton);
	CALL_WRAP(InputSingleton);
	CALL_WRAP(SceneSingleton);
	CALL_WRAP(AppSingleton);
	CALL_WRAP(event_ManagerSingleton);
}
