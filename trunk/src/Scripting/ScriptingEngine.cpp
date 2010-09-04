#include <Python.h>
#include "ScriptingEngine.h"


extern "C" void initAnki(); /// Defined in BoostPythonInterfaces.cpp


//======================================================================================================================
// init                                                                                                                =
//======================================================================================================================
bool ScriptingEngine::init()
{
	INFO("Initializing scripting engine...");

	PyImport_AppendInittab((char*)("Anki"), &initAnki);
	Py_Initialize();
	mainModule = python::object(python::handle<>(python::borrowed(PyImport_AddModule("__main__"))));
	mainNamespace = mainModule.attr("__dict__");
	ankiModule = python::object(python::handle<>(PyImport_ImportModule("Anki")));

	//execScript("import Anki\n");

	INFO("Scripting engine initialized");
	return true;
}


//======================================================================================================================
// execScript                                                                                                          =
//======================================================================================================================
bool ScriptingEngine::execScript(const char* script, const char* scriptName)
{
	try
	{
		python::handle<>ignored(PyRun_String(script, Py_file_input, mainNamespace.ptr(), mainNamespace.ptr()));
	}
	catch(python::error_already_set)
	{
		ERROR("Script \"" << scriptName << "\" failed with error:");
		PyErr_Print();
		return false;
	}
	return true;
}

