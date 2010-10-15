#ifndef UTIL_H
#define UTIL_H

#include <string>
#include "Vec.h"
#include "StdTypes.h"


/// The namespace contains a few useful functions
namespace Util {

extern int randRange(int min, int max); ///< Pick a random number from min to max
extern uint randRange(uint min, uint max); ///< Pick a random number from min to max
extern float randRange(float min, float max); ///< Pick a random number from min to max
extern double randRange(double min, double max); ///< Pick a random number from min to max
extern float randFloat(float max);

extern std::string readFile(const char* filename); ///< Open a text file and return its contents into a string
extern Vec<std::string> getFileLines(const char* filename); ///< Open a text file and return its lines into a string vector

}

#endif
