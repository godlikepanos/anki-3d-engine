// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Functions.h>
#include <cstdlib>
#include <cmath>
#include <cstring>

namespace anki
{

I32 randRange(I32 min, I32 max)
{
	return (rand() % (max - min + 1)) + min;
}

U32 randRange(U32 min, U32 max)
{
	return (rand() % (max - min + 1)) + min;
}

F32 randRange(F32 min, F32 max)
{
	F32 r = (F32)rand() / (F32)RAND_MAX;
	return min + r * (max - min);
}

F64 randRange(F64 min, F64 max)
{
	F64 r = (F64)rand() / (F64)RAND_MAX;
	return min + r * (max - min);
}

F32 randFloat(F32 max)
{
	F32 r = F32(rand()) / F32(RAND_MAX);
	return r * max;
}

} // end namespace anki
