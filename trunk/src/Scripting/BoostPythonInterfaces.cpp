#include "Common.h"
#include <boost/python.hpp>
#include "Math.h"
#include "Scene.h"
#include "MainRenderer.h"
#include "App.h"


using namespace boost::python;
using namespace M;


BOOST_PYTHON_MODULE(Anki)
{
	#include "Vec2.bpi.h"
	#include "Vec3.bpi.h"
	#include "Vec4.bpi.h"

	#include "Scene.bpi.h"

	#include "Hdr.bpi.h"
	#include "Pps.bpi.h"
	#include "Renderer.bpi.h"
	#include "MainRenderer.bpi.h"

	#include "App.bpi.h"
}

