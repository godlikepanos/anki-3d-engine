#include "ScriptingCommon.h"
#include "Math.h"


WRAP(Vec4)
{
	class_<Vec4>("Vec4")
		/// @todo Correct this
		/*.def_readwrite("x", &Vec4::x)
		.def_readwrite("y", &Vec4::y)
		.def_readwrite("z", &Vec4::z)
		.def_readwrite("w", &Vec4::w)*/
		// constructors
		.def(init<>())
		.def(init<float>())
		.def(init<float, float, float, float>())
		.def(init<const Vec2&, float, float>())
		.def(init<const Vec3&, float>())
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
