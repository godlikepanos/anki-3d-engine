#include "Common.h"
#include <boost/python.hpp>
#include "Math.h"
#include "Scene.h"
#include "App.h"


using namespace boost::python;
using namespace M;


BOOST_PYTHON_MODULE(Anki)
{
	#include "Vec2.bpi.h"
	#include "Vec3.bpi.h"
	#include "Vec4.bpi.h"
	#include "Scene.bpi.h"
	#include "App.bpi.h"
}

