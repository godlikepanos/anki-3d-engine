#include "anki/script/MathCommon.h"
#include "anki/Math.h"

namespace anki {

#if 0

static void vec4SetX(Vec4* self, F32 x)
{
	self->x() = x;
}

static void vec4SetY(Vec4* self, F32 y)
{
	self->y() = y;
}

static void vec4SetZ(Vec4* self, F32 z)
{
	self->z() = z;
}

static void vec4SetW(Vec4* self, F32 w)
{
	self->w() = w;
}

static void vec4SetAt(Vec4* self, int i, F32 f)
{
	(*self)[i] = f;
}

ANKI_SCRIPT_WRAP(Vec4)
{
	ANKI_LUA_CLASS_BEGIN(lb, Vec4)
		ANKI_LUA_CONSTRUCTOR(F32, F32, F32, F32)
		ANKI_LUA_METHOD("copy", &Vec4::operator=)
		// Accessors
		ANKI_LUA_METHOD("getX", (F32 (Vec4::*)() const)(&Vec4::x))
		ANKI_LUA_METHOD("getY", (F32 (Vec4::*)() const)(&Vec4::y))
		ANKI_LUA_METHOD("getZ", (F32 (Vec4::*)() const)(&Vec4::z))
		ANKI_LUA_METHOD("getW", (F32 (Vec4::*)() const)(&Vec4::w))
		ANKI_LUA_FUNCTION_AS_METHOD("setX", &vec4SetX)
		ANKI_LUA_FUNCTION_AS_METHOD("setY", &vec4SetY)
		ANKI_LUA_FUNCTION_AS_METHOD("setZ", &vec4SetZ)
		ANKI_LUA_FUNCTION_AS_METHOD("setW", &vec4SetW)
		ANKI_LUA_METHOD("getAt",
			(F32 (Vec4::*)(const U) const)(&Vec4::operator[]))
		ANKI_LUA_FUNCTION_AS_METHOD("setAt", &vec4SetAt)
		// Ops with same
		ANKI_LUA_METHOD("__add",
			(Vec4 (Vec4::*)(const Vec4&) const)(&Vec4::operator+))
		ANKI_LUA_METHOD("__sub",
			(Vec4 (Vec4::*)(const Vec4&) const)(&Vec4::operator-))
		ANKI_LUA_METHOD("__mul",
			(Vec4 (Vec4::*)(const Vec4&) const)(&Vec4::operator*))
		ANKI_LUA_METHOD("__div",
			(Vec4 (Vec4::*)(const Vec4&) const)(&Vec4::operator/))
		ANKI_LUA_METHOD("__eq", &Vec4::operator==)
		ANKI_LUA_METHOD("__lt", &Vec4::operator<)
		ANKI_LUA_METHOD("__le", &Vec4::operator<=)
		// Other
		ANKI_LUA_METHOD("getLength", &Vec4::getLength)
		ANKI_LUA_METHOD("getNormalized", &Vec4::getNormalized)
		ANKI_LUA_METHOD("normalize", &Vec4::normalize)
		ANKI_LUA_METHOD("dot", &Vec4::dot)
	ANKI_LUA_CLASS_END()
}
#endif

} // end namespace anki
