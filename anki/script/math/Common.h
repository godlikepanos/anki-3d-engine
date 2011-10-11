#ifndef ANKI_SCRIPT_MATH_COMMON_H
#define ANKI_SCRIPT_MATH_COMMON_H

#include "anki/script/Common.h"


template<typename ClassType, typename RetType,
	RetType (ClassType::* accessor)() const>
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


#endif
