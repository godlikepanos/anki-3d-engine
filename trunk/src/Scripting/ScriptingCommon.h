/// @file
/// This file is included by all the *.bpi.cpp files
#include <boost/python.hpp>


using namespace boost;
using namespace boost::python;


/// Wrap a class
#define WRAP(x) \
	void boostPythonWrap##x()


/// Wrap a singleton class
#define WRAP_SINGLETON(x) \
	WRAP(x) { \
		class_<x, noncopyable>(#x, no_init) \
			.def("getInstance", & x ::getInstance, return_value_policy<reference_existing_object>()) \
			.staticmethod("getInstance") \
		; \
	}


/// Boost python property read write
#define BP_PROPERTY_RW(name__, getter__, setter__) \
	.add_property(name__, & getter__##Value, & setter__##Value)
