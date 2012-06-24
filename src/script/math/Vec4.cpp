#include "anki/script/MathCommon.h"
#include "anki/math/Math.h"

ANKI_WRAP(Vec4)
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
		ANKI_BP_PROPERTY_MATH(Vec4, x)
		ANKI_BP_PROPERTY_MATH(Vec4, y)
		ANKI_BP_PROPERTY_MATH(Vec4, z)
		ANKI_BP_PROPERTY_MATH(Vec4, w)
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
