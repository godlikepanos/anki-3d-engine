#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include "Util.h"
#include "Exception.h"


using namespace std;


namespace Util {


//======================================================================================================================
// randRange                                                                                                           =
//======================================================================================================================
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


//======================================================================================================================
// readFile                                                                                                            =
//======================================================================================================================
string readFile(const char* filename)
{
	ifstream file(filename);
	if (!file.is_open())
	{
		THROW_EXCEPTION(string("Cannot open file \"") + filename + "\"");
	}

	return string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
}


//======================================================================================================================
// getFileLines                                                                                                        =
//======================================================================================================================
Vec<string> getFileLines(const char* filename)
{
	Vec<string> lines;
	ifstream ifs(filename);
	if(!ifs.is_open())
	{
		THROW_EXCEPTION(string("Cannot open file \"") + filename + "\"");
	}

  string temp;
  while(getline(ifs, temp))
  {
  	lines.push_back(temp);
  }
  return lines;
}


//======================================================================================================================
// randFloat                                                                                                           =
//======================================================================================================================
float randFloat(float max)
{
	float r = float(rand()) / float(RAND_MAX);
	return r * max;
}


} // end namesapce
