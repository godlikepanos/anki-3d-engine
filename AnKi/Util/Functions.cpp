// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Util/Functions.h>
#include <AnKi/Util/HighRezTimer.h>
#include <random>

namespace anki {

static std::random_device g_rd;
thread_local static std::mt19937_64 g_randromGenerator(g_rd());

U64 getRandom()
{
	return g_randromGenerator();
}

} // end namespace anki
