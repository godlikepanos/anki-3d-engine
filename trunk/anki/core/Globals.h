#ifndef ANKI_CORE_GLOBALS_H
#define ANKI_CORE_GLOBALS_H

#include "anki/util/Singleton.h"


namespace anki {


typedef Singleton<class Logger> LoggerSingleton;
typedef Singleton<class MainRenderer> MainRendererSingleton;
typedef Singleton<class Input> InputSingleton;
typedef Singleton<class Scene> SceneSingleton;
typedef Singleton<class App> AppSingleton;
typedef Singleton<class StdinListener> StdinListenerSingleton;
typedef Singleton<class GlStateMachine> GlStateMachineSingleton;
typedef Singleton<class ScriptManager> ScriptManagerSingleton;
typedef Singleton<class EventManager> EventManagerSingleton;
typedef Singleton<class ParallelManager> ParallelManagerSingleton;


} // end namespace


#endif
