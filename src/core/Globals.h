#ifndef GLOBALS_H
#define GLOBALS_H

#include "util/Singleton.h"


typedef Singleton<class Logger> LoggerSingleton;
typedef Singleton<class MainRenderer> MainRendererSingleton;
typedef Singleton<class Input> InputSingleton;
typedef Singleton<class ResourceManager> ResourceManagerSingleton;
typedef Singleton<class Scene> SceneSingleton;
typedef Singleton<class App> AppSingleton;
typedef Singleton<class StdinListener> StdinListenerSingleton;
typedef Singleton<class GlStateMachine> GlStateMachineSingleton;

namespace script {
typedef Singleton<class Engine> EngineSingleton;
}

typedef Singleton<class EventManager> EventManagerSingleton;

namespace parallel {
typedef Singleton<class Manager> ManagerSingleton;
}


#endif
