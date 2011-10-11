#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include "anki/util/Util.h"
#include "anki/util/Exception.h"


namespace util {


//==============================================================================
// randRange                                                                   =
//==============================================================================
int randRange(int min, int max)
{
	return (rand() % (max-min+1)) + min ;
}

uint randRange(uint min, uint max)
{
	return (rand() % (max-min+1)) + min ;
}

float randRange(float min, float max)
{
	float r = (float)rand() / (float)RAND_MAX;
	return min + r * (max - min);
}

double randRange(double min, double max)
{
	double r = (double)rand() / (double)RAND_MAX;
	return min + r * (max - min);
}


//==============================================================================
// readFile                                                                    =
//==============================================================================
std::string readFile(const char* filename)
{
	std::ifstream file(filename);
	if (!file.is_open())
	{
		throw ANKI_EXCEPTION(std::string("Cannot open file \"") + filename + "\"");
	}

	return std::string((std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>());
}


//==============================================================================
// getFileLines                                                                =
//==============================================================================
std::vector<std::string> getFileLines(const char* filename)
{
	std::vector<std::string> lines;
	std::ifstream ifs(filename);
	if(!ifs.is_open())
	{
		throw ANKI_EXCEPTION("Cannot open file \"" + filename + "\"");
	}

  std::string temp;
  while(getline(ifs, temp))
  {
  	lines.push_back(temp);
  }
  return lines;
}


//==============================================================================
// randFloat                                                                   =
//==============================================================================
float randFloat(float max)
{
	float r = float(rand()) / float(RAND_MAX);
	return r * max;
}


} // end namesapce
