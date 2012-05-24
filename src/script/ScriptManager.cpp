#include "anki/script/ScriptManager.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/core/Globals.h"
#include "anki/script/Common.h"
#include <Python.h>


using namespace boost;
using namespace boost::python;


/// Define the classes
BOOST_PYTHON_MODULE(anki)
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

	CALL_WRAP(SwitchableRenderingPass);
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


namespace anki {


//==============================================================================
void ScriptManager::init()
{
	ANKI_LOGI("Initializing scripting engine...");

	PyImport_AppendInittab((char*)("anki"), &initanki);
	Py_Initialize();
	mainModule = object(handle<>(borrowed(PyImport_AddModule("__main__"))));
	mainNamespace = mainModule.attr("__dict__");
	ankiModule = object(handle<>(PyImport_ImportModule("anki")));

	ANKI_LOGI("Scripting engine initialized");
}


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
		throw ANKI_EXCEPTION("Script \"" + scriptName + "\" failed with error");
	}
}


} // end namespace
