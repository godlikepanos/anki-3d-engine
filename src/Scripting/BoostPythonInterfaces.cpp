#include <boost/python.hpp>
#include "ScriptingCommon.h"


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

	CALL_WRAP(EventSceneColor);
	CALL_WRAP(EventMainRendererPpsHdr);
	CALL_WRAP(EventManager);

	CALL_WRAP(App);

	CALL_WRAP(Input);

	CALL_WRAP(AllGlobals);
}
