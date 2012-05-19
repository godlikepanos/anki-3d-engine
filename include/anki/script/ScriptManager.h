#ifndef ANKI_SCRIPT_SCRIPT_MANAGER_H
#define ANKI_SCRIPT_SCRIPT_MANAGER_H

#include <boost/python.hpp>


namespace anki {


/// The scripting manager using Python
class ScriptManager
{
public:
	ScriptManager()
	{
		init();
	}

	/// Execute python script
	/// @param script Script source
	/// @return true on success
	void execScript(const char* script, const char* scriptName = "unamed");

	/// Expose a C++ variable to python
	/// @param varName The name to referenced in python
	/// @param var The actual variable
	template<typename Type>
	void exposeVar(const char* varName, Type* var)
	{
		using namespace boost::python;
		scope(ankiModule).attr(varName) = ptr(var);
	}

private:
	boost::python::object mainModule;
	boost::python::object ankiModule;
	boost::python::object mainNamespace;

	void init();
};


} // end namespace


#endif
