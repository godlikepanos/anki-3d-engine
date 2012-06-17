#include "anki/util/Functions.h"
#include <cstdlib>
#include <cmath>
#include <cstring>

namespace anki {

//==============================================================================
int randRange(int min, int max)
{
	return (rand() % (max - min + 1)) + min;
}

//==============================================================================
uint32_t randRange(uint32_t min, uint32_t max)
{
	return (rand() % (max - min + 1)) + min;
}

//==============================================================================
float randRange(float min, float max)
{
	float r = (float)rand() / (float)RAND_MAX;
	return min + r * (max - min);
}

//==============================================================================
double randRange(double min, double max)
{
	double r = (double)rand() / (double)RAND_MAX;
	return min + r * (max - min);
}

//==============================================================================
float randFloat(float max)
{
	float r = float(rand()) / float(RAND_MAX);
	return r * max;
}

} // end namespace anki
