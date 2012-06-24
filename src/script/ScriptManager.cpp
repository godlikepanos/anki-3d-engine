#include "anki/script/ScriptManager.h"
#include "anki/util/Exception.h"
#include "anki/core/Logger.h"
#include "anki/script/Common.h"
#include <Python.h>

using namespace boost;
using namespace boost::python;

/// Define the classes
BOOST_PYTHON_MODULE(anki)
{
	ANKI_CALL_WRAP(HighRezTimer);

	ANKI_CALL_WRAP(Vec2);
	ANKI_CALL_WRAP(Vec3);
	ANKI_CALL_WRAP(Vec4);

	ANKI_CALL_WRAP(Logger);

	ANKI_CALL_WRAP(SceneNode);
	ANKI_CALL_WRAP(Camera);
	ANKI_CALL_WRAP(MaterialRuntimeVariable);
	ANKI_CALL_WRAP(MaterialRuntime);
	ANKI_CALL_WRAP(PatchNode);
	ANKI_CALL_WRAP(ModelPatchNode);
	ANKI_CALL_WRAP(ModelNode);
	ANKI_CALL_WRAP(Scene);

	ANKI_CALL_WRAP(SwitchableRenderingPass);
	ANKI_CALL_WRAP(Hdr);
	ANKI_CALL_WRAP(Bl);
	ANKI_CALL_WRAP(Pps);
	ANKI_CALL_WRAP(Renderer);
	ANKI_CALL_WRAP(Dbg);
	ANKI_CALL_WRAP(MainRenderer);

	ANKI_CALL_WRAP(SceneColorEvent);
	ANKI_CALL_WRAP(MainRendererPpsHdrEvent);
	ANKI_CALL_WRAP(EventManager);

	ANKI_CALL_WRAP(App);

	ANKI_CALL_WRAP(Input);

	ANKI_CALL_WRAP(AllGlobals);
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
