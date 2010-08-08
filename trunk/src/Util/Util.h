#ifndef _UTIL_H_
#define _UTIL_H_

#include "Common.h"
#include "Vec.h"

/**
 * The namespace contains a few useful functions
 */
namespace Util {

extern int    randRange(int min, int max); ///< Pick a random number from min to max
extern uint   randRange(uint min, uint max); ///< Pick a random number from min to max
extern float  randRange(float min, float max); ///< Pick a random number from min to max
extern double randRange(double min, double max); ///< Pick a random number from min to max
extern float randFloat(float max);

extern string      readFile(const char* filename); ///< Open a text file and return its contents into a string
extern Vec<string> getFileLines(const char* filename); ///< Open a text file and return its lines into a string vector

}

#endif
