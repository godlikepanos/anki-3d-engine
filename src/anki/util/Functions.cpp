// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/util/Functions.h>
#include <anki/util/HighRezTimer.h>
#include <random>

namespace anki
{

thread_local std::mt19937_64 g_randromGenerator(U64(HighRezTimer::getCurrentTime() * 1000000.0));

U64 getRandom()
{
	return g_randromGenerator();
}

} // end namespace anki
