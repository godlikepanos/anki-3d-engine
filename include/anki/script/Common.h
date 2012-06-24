#ifndef ANKI_SCRIPT_COMMON_H
#define ANKI_SCRIPT_COMMON_H

/// @file
/// This file is included by all the *.bpi.cpp files
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

namespace anki
{}

using namespace boost;
using namespace boost::python;
using namespace anki;

/// Wrap a class
#define ANKI_WRAP(x) \
	void boostPythonWrap##x()

/// Wrap a singleton class
#define ANKI_WRAP_SINGLETON(x) \
	WRAP(x) { \
		class_<x, noncopyable>(#x, no_init) \
			.def("get", & x ::get, \
				return_value_policy<reference_existing_object>()) \
			.staticmethod("get") \
		; \
	}

#define ANKI_WRAP_CONTAINER(x) \
	class_<x >(#x) \
		.def(vector_indexing_suite<x >()) \
	;

#define ANKI_CALL_WRAP(x) extern void boostPythonWrap##x(); boostPythonWrap##x()

//==============================================================================
// Property for simple types                                                   =
//==============================================================================

template<typename ClassType, typename RetType,
	const RetType& (ClassType::* accessor)() const>
RetType getterSv(const ClassType* t)
{
	return (t->*accessor)();
}

template<typename ClassType, typename InType,
	void (ClassType::* accessor)(const InType&)>
void setterSv(ClassType* t, InType in)
{
	(t->*accessor)(in);
}

/// Boost python property for simple types (int, float etc) that cannot be
/// wrapped by boost::python correctly
#define BP_PROPERTY_BASIC_TYPE(Type__, Class__, var__, getter__, setter__) \
	.add_property(#var__, &getterSv<Class__, Type__, &Class__::getter__>, \
		&setterSv<Class__, Type__, &Class__::setter__>)

#endif
