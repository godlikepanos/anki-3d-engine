#include "anki/script/math/Common.h"
#include "anki/math/Math.h"


WRAP(Vec4)
{
	class_<Vec4>("Vec4")
		// constructors
		.def(init<>())
		.def(init<float>())
		.def(init<float, float, float, float>())
		.def(init<const Vec2&, float, float>())
		.def(init<const Vec3&, float>())
		.def(init<const Vec4&>())
		.def(init<const Quat&>())
		// Accessors
		BP_PROPERTY_MATH(Vec4, x)
		BP_PROPERTY_MATH(Vec4, y)
		BP_PROPERTY_MATH(Vec4, z)
		BP_PROPERTY_MATH(Vec4, w)
		// ops with self
		.def(self + self) // +
		.def(self += self) // +=
		.def(self - self) // -
		.def(self -= self) // -=
		.def(self * self) // *
		.def(self *= self) // *=
		.def(self / self) // /
		.def(self /= self) // /=
		.def(- self) // negative
		.def(self == self) // ==
		.def(self != self) // ==
		// ops with float
		.def(self + float()) // +
		.def(self += float()) // +=
		.def(self - float()) // -
		.def(self -= float()) // -=
		.def(self * float()) // *
		.def(self *= float()) // *=
		.def(self / float()) // /
		.def(self /= float()) // /=
	;
}
