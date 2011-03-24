#include <boost/python.hpp>
#include "App.h"


#define CALL_WRAP(x) extern void boostPythonWrap##x(); boostPythonWrap##x()


BOOST_PYTHON_MODULE(Anki)
{
	CALL_WRAP(Vec2);
	CALL_WRAP(Vec3);
	CALL_WRAP(Vec4);

	CALL_WRAP(Logger);
	CALL_WRAP(LoggerSingleton);

	CALL_WRAP(SceneNode);
	CALL_WRAP(Camera);
	CALL_WRAP(Scene);
	CALL_WRAP(SceneSingleton);

	CALL_WRAP(Hdr);
	CALL_WRAP(Pps);
	CALL_WRAP(Renderer);
	CALL_WRAP(Dbg);
	CALL_WRAP(MainRenderer);
	CALL_WRAP(MainRendererSingleton);

	CALL_WRAP(App);
	CALL_WRAP(AppSingleton);
}
