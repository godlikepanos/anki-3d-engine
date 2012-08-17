#include "anki/script/MathCommon.h"
#include "anki/math/Math.h"

namespace anki {

static float vec2GetX(Vec2* self)
{
	return self->x();
}

static void vec2SetX(Vec2* self, float x)
{
	self->x() = x;
}

static float vec2GetY(Vec2* self)
{
	return self->y();
}

static void vec2SetY(Vec2* self, float y)
{
	self->y() = y;
}

static float vec2GetAt(Vec2* self, int i)
{
	return (*self)[i];
}

ANKI_SCRIPT_WRAP(Vec2)
{
	ANKI_LUA_CLASS_BEGIN(lb, Vec2)
		ANKI_LUA_CONSTRUCTOR(float, float)
		// Accessors
		ANKI_LUA_FUNCTION_AS_METHOD("getX", &vec2GetX)
		ANKI_LUA_FUNCTION_AS_METHOD("getY", &vec2GetY)
		ANKI_LUA_FUNCTION_AS_METHOD("setX", &vec2SetX)
		ANKI_LUA_FUNCTION_AS_METHOD("setY", &vec2SetY)
		ANKI_LUA_FUNCTION_AS_METHOD("getAt", &vec2GetAt)
		// Ops with same
		ANKI_LUA_METHOD("add", 
			(Vec2 (Vec2::*)(const Vec2&) const)(&Vec2::operator+))
	ANKI_LUA_CLASS_END()
}

} // end namespace anki
