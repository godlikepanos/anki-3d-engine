#ifndef GLOBALS_H
#define GLOBALS_H

#include "Singleton.h"


typedef Singleton<class Logger> LoggerSingleton;
typedef Singleton<class MainRenderer> MainRendererSingleton;
typedef Singleton<class Input> InputSingleton;
typedef Singleton<class ResourceManager> ResourceManagerSingleton;
typedef Singleton<class Scene> SceneSingleton;
typedef Singleton<class App> AppSingleton;
typedef Singleton<class StdinListener> StdinListenerSingleton;
typedef Singleton<class GlStateMachine> GlStateMachineSingleton;
typedef Singleton<class ScriptingEngine> ScriptingEngineSingleton;

namespace Event {
typedef Singleton<class Manager> ManagerSingleton;
}

typedef Singleton<class JobManager> JobManagerSingleton;


#endif
