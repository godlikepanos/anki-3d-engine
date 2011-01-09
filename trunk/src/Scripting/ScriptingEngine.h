#ifndef SCRIPTING_ENGINE_H
#define SCRIPTING_ENGINE_H

#include <boost/python.hpp>
#include "Singleton.h"


/// The scripting engine using Python
class ScriptingEngine
{
	public:
		ScriptingEngine() {init();}

		/// Execute python script
		/// @param script Script source
		/// @return true on success
		void execScript(const char* script, const char* scriptName = "unamed");

		/// Expose a C++ variable to python
		/// @param varName The name to referenced in python
		/// @param var The actual variable
		template<typename Type>
		void exposeVar(const char* varName, Type* var);

	private:
		static ScriptingEngine* instance;
		boost::python::object mainModule;
		boost::python::object ankiModule;
		boost::python::object mainNamespace;

		void init();
};


template<typename Type>
inline void ScriptingEngine::exposeVar(const char* varName, Type* var)
{
	boost::python::scope(ankiModule).attr(varName) = boost::python::ptr(var);
}


typedef Singleton<ScriptingEngine> ScriptingEngineSingleton;


#endif
