#include "anki/script/MathCommon.h"
#include "anki/math/Math.h"

ANKI_WRAP(Vec2)
{
	class_<Vec2>("Vec2")
		// constructors
		.def(init<>())
		.def(init<float>())
		.def(init<float, float>())
		.def(init<const Vec2&>())
		.def(init<const Vec3&>())
		.def(init<const Vec4&>())
		// Accessors
		ANKI_BP_PROPERTY_MATH(Vec2, x)
		ANKI_BP_PROPERTY_MATH(Vec2, y)
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
