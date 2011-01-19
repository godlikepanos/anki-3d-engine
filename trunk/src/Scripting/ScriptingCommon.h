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


//======================================================================================================================
// Property for simple types                                                                                           =
//======================================================================================================================

template<typename ClassType, typename RetType, const RetType& (ClassType::* accessor)() const>
RetType getterSv(const ClassType* t)
{
	return (t->*accessor)();
}


template<typename ClassType, typename InType, void (ClassType::* accessor)(const InType&)>
void setterSv(ClassType* t, InType in)
{
	(t->*accessor)(in);
}

/// Boost python property for simple types (int, float etc) that cannot be wrapped by boost::python correctly
#define BP_PROPERTY_BASIC_TYPE(Type__, Class__, var__, getter__, setter__) \
	.add_property(var__, &getterSv<Class__, Type__, &Class__::getter__>, \
	              &setterSv<Class__, Type__, &Class__::setter__>)


//======================================================================================================================
// Math library stuff                                                                                                  =
//======================================================================================================================

template<typename ClassType, typename RetType, RetType (ClassType::* accessor)() const>
RetType getM(const ClassType* t)
{
	return (t->*accessor)();
}


template<typename ClassType, typename InType, InType& (ClassType::* accessor)()>
void setM(ClassType* t, InType in)
{
	(t->*accessor)() = in;
}


#define BP_PROPERTY_MATH(ClassType__, name__) \
	.add_property(#name__, &getM<ClassType__, float, &ClassType__::name__>, \
	              &setM<ClassType__, float, &ClassType__::name__>)

