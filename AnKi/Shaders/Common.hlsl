// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// This file contains common code for all shaders. It's optional but it's recomended to include it

#pragma once

#include <AnKi/Shaders/Include/Common.h>

template<typename T>
T uvToNdc(T x)
{
	return x * 2.0f - 1.0f;
}

template<typename T>
T ndcToUv(T x)
{
	return x * 0.5f + 0.5f;
}
