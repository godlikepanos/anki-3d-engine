#include "ScriptingCommon.h"
#include "Math.h"


WRAP(Vec3)
{
	class_<Vec3>("Vec3")
		/// @todo correct
		/*.def_readwrite("x", &Vec3::x)
		.def_readwrite("y", &Vec3::y)
		.def_readwrite("z", &Vec3::z)*/
		// constructors
		.def(init<>())
		.def(init<float>())
		.def(init<float, float, float>())
		.def(init<const Vec2&, float>())
		.def(init<const Vec3&>())
		.def(init<const Vec4&>())
		.def(init<const Quat&>())
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
