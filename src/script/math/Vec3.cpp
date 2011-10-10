#include "ScriptMCommon.h"
#include "math/Math.h"


WRAP(Vec3)
{
	class_<Vec3>("Vec3")
		// Constructors
		.def(init<>())
		.def(init<float>())
		.def(init<float, float, float>())
		.def(init<const Vec2&, float>())
		.def(init<const Vec3&>())
		.def(init<const Vec4&>())
		.def(init<const Quat&>())
		// Accessors
		BP_PROPERTY_MATH(Vec3, x)
		BP_PROPERTY_MATH(Vec3, y)
		BP_PROPERTY_MATH(Vec3, z)
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
