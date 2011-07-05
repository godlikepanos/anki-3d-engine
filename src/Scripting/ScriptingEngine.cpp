#include <Python.h>
#include "ScriptingEngine.h"
#include "Util/Exception.h"
#include "Core/Logger.h"
#include "Core/Globals.h"


/// Defined in BoostPythonInterfaces.cpp by boost::python
extern "C" void initAnki();


//==============================================================================
// init                                                                        =
//==============================================================================
void ScriptingEngine::init()
{
	INFO("Initializing scripting engine...");

	PyImport_AppendInittab((char*)("Anki"), &initAnki);
	Py_Initialize();
	mainModule = boost::python::object(boost::python::handle<>(
		boost::python::borrowed(PyImport_AddModule("__main__"))));
	mainNamespace = mainModule.attr("__dict__");
	ankiModule = boost::python::object(
		boost::python::handle<>(PyImport_ImportModule("Anki")));

	INFO("Scripting engine initialized");
}


//==============================================================================
// execScript                                                                  =
//==============================================================================
void ScriptingEngine::execScript(const char* script, const char* scriptName)
{
	try
	{
		boost::python::handle<>ignored(PyRun_String(script, Py_file_input,
			mainNamespace.ptr(), mainNamespace.ptr()));
	}
	catch(boost::python::error_already_set)
	{
		PyErr_Print();
		throw EXCEPTION("Script \"" + scriptName + "\" failed with error");
	}
}

