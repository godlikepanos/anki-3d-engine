#ifndef UTIL_H
#define UTIL_H

#include "util/StdTypes.h"
#include <string>
#include <vector>


/// The namespace contains a few useful functions
namespace util {

/// Pick a random number from min to max
extern int randRange(int min, int max);

/// Pick a random number from min to max
extern uint randRange(uint min, uint max);

/// Pick a random number from min to max
extern float randRange(float min, float max);

/// Pick a random number from min to max
extern double randRange(double min, double max);

extern float randFloat(float max);

/// Open a text file and return its contents into a string
extern std::string readFile(const char* filename);

/// Open a text file and return its lines into a string vector
extern std::vector<std::string> getFileLines(const char* filename);

/// Get the size in bytes of a vector
template<typename Vec>
size_t getVectorSizeInBytes(const Vec& v)
{
	return v.size() * sizeof(typename Vec::value_type);
}


} // end namespace


#endif
