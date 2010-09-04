#include "Common.h"
#include <boost/python.hpp>
#include "Math.h"
#include "Scene.h"
#include "App.h"


using namespace boost::python;
using namespace M;


BOOST_PYTHON_MODULE(Anki)
{
	#include "Vec3.bpi.h"
	#include "Scene.bpi.h"
	#include "App.bpi.h"
}

