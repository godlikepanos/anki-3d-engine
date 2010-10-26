/// @file
/// This file is included by all the *.bpi.cpp files

#include <boost/python.hpp>

using namespace boost;
using namespace boost::python;

#define WRAP(x) \
	void boostPythonWrap##x()
