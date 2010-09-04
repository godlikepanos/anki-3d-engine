#ifndef SCRIPTING_ENGINE_H
#define SCRIPTING_ENGINE_H

#include <boost/python.hpp>
#include "Common.h"
#include "Object.h"


/**
 *
 */
class ScriptingEngine: public Object
{
	public:
		ScriptingEngine(Object* parent = NULL);
		~ScriptingEngine() {}

		/**
		 * Execute python script
		 * @param script Script source
		 * @return true on success
		 */
		bool execScript(const char* script, const char* scriptName = "unamed");

		/**
		 * Expose a C++ variable to python
		 * @param varName The name to referenced in python
		 * @param var The actual variable
		 */
		template<typename Type>
		void exposeVar(const char* varName, Type* var);

	private:
		boost::python::object mainModule;
		boost::python::object ankiModule;
		boost::python::object mainNamespace;

		/**
		 * Init python and modules
		 */
		bool init();
};


inline ScriptingEngine::ScriptingEngine(Object* parent):
	Object(parent)
{
	init();
}


template<typename Type>
inline void ScriptingEngine::exposeVar(const char* varName, Type* var)
{
	python::scope(ankiModule).attr(varName) = python::ptr(var);
}


#endif
