#ifndef ANKI_UTIL_UTIL_H
#define ANKI_UTIL_UTIL_H

#include "anki/util/StdTypes.h"
#include <string>
#include <vector>


namespace anki {


/// Contains a few useful functions
class Util
{
	public:
		/// Pick a random number from min to max
		static int randRange(int min, int max);

		/// Pick a random number from min to max
		static uint randRange(uint min, uint max);

		/// Pick a random number from min to max
		static float randRange(float min, float max);

		/// Pick a random number from min to max
		static double randRange(double min, double max);

		static float randFloat(float max);

		/// Open a text file and return its contents into a string
		static std::string readFile(const char* filename);

		/// Open a text file and return its lines into a string vector
		static std::vector<std::string> getFileLines(const char* filename);

		/// Get the size in bytes of a vector
		template<typename Vec>
		static size_t getVectorSizeInBytes(const Vec& v)
		{
			return v.size() * sizeof(typename Vec::value_type);
		}
};


} // end namespace


#endif
