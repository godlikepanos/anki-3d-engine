#include "anki/script/MathCommon.h"
#include "anki/math/Math.h"

namespace anki {

static void vec3SetX(Vec3* self, float x)
{
	self->x() = x;
}

static void vec3SetY(Vec3* self, float y)
{
	self->y() = y;
}

static void vec3SetZ(Vec3* self, float z)
{
	self->z() = z;
}

static void vec3SetAt(Vec3* self, int i, float f)
{
	(*self)[i] = f;
}

ANKI_SCRIPT_WRAP(Vec3)
{
	ANKI_LUA_CLASS_BEGIN(lb, Vec3)
		ANKI_LUA_CONSTRUCTOR(float, float, float)
		ANKI_LUA_METHOD("copy", &Vec3::operator=)
		// Accessors
		ANKI_LUA_METHOD("getX", (float (Vec3::*)() const)(&Vec3::x))
		ANKI_LUA_METHOD("getY", (float (Vec3::*)() const)(&Vec3::y))
		ANKI_LUA_METHOD("getZ", (float (Vec3::*)() const)(&Vec3::z))
		ANKI_LUA_FUNCTION_AS_METHOD("setX", &vec3SetX)
		ANKI_LUA_FUNCTION_AS_METHOD("setY", &vec3SetY)
		ANKI_LUA_FUNCTION_AS_METHOD("setZ", &vec3SetZ)
		ANKI_LUA_METHOD("getAt",
			(float (Vec3::*)(const size_t) const)(&Vec3::operator[]))
		ANKI_LUA_FUNCTION_AS_METHOD("setAt", &vec3SetAt)
		// Ops with same
		ANKI_LUA_METHOD("__add",
			(Vec3 (Vec3::*)(const Vec3&) const)(&Vec3::operator+))
		ANKI_LUA_METHOD("__sub",
			(Vec3 (Vec3::*)(const Vec3&) const)(&Vec3::operator-))
		ANKI_LUA_METHOD("__mul",
			(Vec3 (Vec3::*)(const Vec3&) const)(&Vec3::operator*))
		ANKI_LUA_METHOD("__div",
			(Vec3 (Vec3::*)(const Vec3&) const)(&Vec3::operator/))
		ANKI_LUA_METHOD("__eq", &Vec3::operator==)
		ANKI_LUA_METHOD("__lt", &Vec3::operator<)
		ANKI_LUA_METHOD("__le", &Vec3::operator<=)
		// Other
		ANKI_LUA_METHOD("getLength", &Vec3::getLength)
		ANKI_LUA_METHOD("getNormalized", &Vec3::getNormalized)
		ANKI_LUA_METHOD("normalize", &Vec3::normalize)
		ANKI_LUA_METHOD("dot", &Vec3::dot)
	ANKI_LUA_CLASS_END()
}

} // end namespace anki
