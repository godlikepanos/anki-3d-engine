#include "ScriptingCommon.h"
#include "Math.h"


WRAP(Vec2)
{
	class_<Vec2>("Vec2")
		.def_readwrite("x", &Vec2::x)
		.def_readwrite("y", &Vec2::y)
		// constructors
		.def(init<>())
		.def(init<float>())
		.def(init<float, float>())
		.def(init<const Vec2&>())
		.def(init<const Vec3&>())
		.def(init<const Vec4&>())
		// ops with same type
		.def(self + self)
		.def(self += self)
		.def(self - self)
		.def(self -= self)
		.def(self * self)
		.def(self *= self)
		.def(self / self)
		.def(self /= self)
		.def(- self)
		.def(self == self)
		.def(self != self)
		// ops with float
		.def(self + float())
		.def(self += float())
		.def(self - float())
		.def(self -= float())
		.def(self * float())
		.def(self *= float())
		.def(self / float())
		.def(self /= float())
		// other
		.def("getLength", &Vec2::getLength)
		.def("getNormalized", &Vec2::getNormalized)
		.def("normalize", &Vec2::normalize)
		.def("dot", &Vec2::dot)
	;
}
