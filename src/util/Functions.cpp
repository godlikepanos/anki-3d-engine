#include "anki/util/Functions.h"
#include <cstdlib>
#include <cmath>
#include <cstring>

namespace anki {

//==============================================================================
I32 randRange(I32 min, I32 max)
{
	return (rand() % (max - min + 1)) + min;
}

//==============================================================================
U32 randRange(U32 min, U32 max)
{
	return (rand() % (max - min + 1)) + min;
}

//==============================================================================
F32 randRange(F32 min, F32 max)
{
	F32 r = (F32)rand() / (F32)RAND_MAX;
	return min + r * (max - min);
}

//==============================================================================
F64 randRange(F64 min, F64 max)
{
	F64 r = (F64)rand() / (F64)RAND_MAX;
	return min + r * (max - min);
}

//==============================================================================
F32 randFloat(F32 max)
{
	F32 r = F32(rand()) / F32(RAND_MAX);
	return r * max;
}

//==============================================================================
std::string replaceAllString(const std::string& str, const std::string& from, 
	const std::string& to)
{
	if(from.empty())
	{
		return str;
	}

	std::string out = str;
	size_t start_pos = 0;
	while((start_pos = out.find(from, start_pos)) != std::string::npos) 
	{
		out.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}

	return out;
}

} // end namespace anki
