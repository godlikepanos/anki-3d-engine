// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/math/CommonSrc.h>

namespace anki
{

/// @memberof TVec4
template<typename T>
TVec4<T> operator+(const T f, const TVec4<T>& v4)
{
	return v4 + f;
}

/// @memberof TVec4
template<typename T>
TVec4<T> operator-(const T f, const TVec4<T>& v4)
{
	return TVec4<T>(f) - v4;
}

/// @memberof TVec4
template<typename T>
TVec4<T> operator*(const T f, const TVec4<T>& v4)
{
	return v4 * f;
}

/// @memberof TVec4
template<typename T>
TVec4<T> operator/(const T f, const TVec4<T>& v4)
{
	return TVec4<T>(f) / v4;
}

} // end namespace anki
