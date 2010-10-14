#ifndef SCRIPTING_ENGINE_H
#define SCRIPTING_ENGINE_H

#include <boost/python.hpp>
#include "Object.h"


/// The scripting engine using Python
class ScriptingEngine: public Object
{
	public:
		/// Default constructor
		ScriptingEngine(Object* parent = NULL);

		/// Destructor
		~ScriptingEngine() {}

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
		boost::python::object mainModule;
		boost::python::object ankiModule;
		boost::python::object mainNamespace;

		/// Init python and modules
		void init();
};


inline ScriptingEngine::ScriptingEngine(Object* parent):
	Object(parent)
{
	init();
}


template<typename Type>
inline void ScriptingEngine::exposeVar(const char* varName, Type* var)
{
	boost::python::scope(ankiModule).attr(varName) = boost::python::ptr(var);
}


#endif
