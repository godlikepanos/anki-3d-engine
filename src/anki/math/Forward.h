// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/StdTypes.h>

namespace anki
{

class F16;

template<typename T, U N, typename TSimd, typename TV>
class TVec;
template<typename T>
class TVec2;
template<typename T>
class TVec3;
template<typename T>
class TVec4;

template<typename T, U J, U I, typename TSimd, typename TM, typename TVJ, typename TVI>
class TMat;
template<typename T>
class TMat3;
template<typename T>
class TMat3x4;
template<typename T>
class TMat4;

template<typename T>
class TQuat;
template<typename T>
class TTransform;
template<typename T>
class TAxisang;
template<typename T>
class TEuler;

} // end namespace anki
