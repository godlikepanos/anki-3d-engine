#include "ScriptManager.h"
#include "util/Exception.h"
#include "core/Logger.h"
#include "core/Globals.h"
#include "ScriptCommon.h"
#include <Python.h>


/// Define the classes
BOOST_PYTHON_MODULE(Anki)
{
	CALL_WRAP(HighRezTimer);

	CALL_WRAP(Vec2);
	CALL_WRAP(Vec3);
	CALL_WRAP(Vec4);

	CALL_WRAP(Logger);

	CALL_WRAP(SceneNode);
	CALL_WRAP(Camera);
	CALL_WRAP(MaterialRuntimeVariable);
	CALL_WRAP(MaterialRuntime);
	CALL_WRAP(PatchNode);
	CALL_WRAP(ModelPatchNode);
	CALL_WRAP(ModelNode);
	CALL_WRAP(Scene);

	CALL_WRAP(Hdr);
	CALL_WRAP(Bl);
	CALL_WRAP(Pps);
	CALL_WRAP(Renderer);
	CALL_WRAP(Dbg);
	CALL_WRAP(MainRenderer);

	CALL_WRAP(SceneColorEvent);
	CALL_WRAP(MainRendererPpsHdrEvent);
	CALL_WRAP(EventManager);

	CALL_WRAP(App);

	CALL_WRAP(Input);

	CALL_WRAP(AllGlobals);
}


using namespace boost::python;


//==============================================================================
// init                                                                        =
//==============================================================================
void ScriptManager::init()
{
	INFO("Initializing scripting engine...");

	PyImport_AppendInittab((char*)("Anki"), &initAnki);
	Py_Initialize();
	mainModule = object(handle<>(borrowed(PyImport_AddModule("__main__"))));
	mainNamespace = mainModule.attr("__dict__");
	ankiModule = object(handle<>(PyImport_ImportModule("Anki")));

	INFO("Scripting engine initialized");
}


//==============================================================================
// execScript                                                                  =
//==============================================================================
void ScriptManager::execScript(const char* script, const char* scriptName)
{
	try
	{
		handle<>ignored(PyRun_String(script, Py_file_input,
			mainNamespace.ptr(), mainNamespace.ptr()));
	}
	catch(error_already_set)
	{
		PyErr_Print();
		throw EXCEPTION("Script \"" + scriptName + "\" failed with error");
	}
}

