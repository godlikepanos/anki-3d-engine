#include <cstdlib>
#include <cmath>
#include <cstring>
#include <fstream>
#include "Util.h"


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
		ERROR("Cannot open file \"" << filename << "\"");
		return string();
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
		ERROR("Cannot open file \"" << filename << "\"");
		return lines;
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


//======================================================================================================================
// readStringFromBinaryStream                                                                                          =
//======================================================================================================================
bool readStringFromBinaryStream(istream& s, string& out, ByteOrder byteOrder)
{
	uchar c[4];
	s.read((char*)c, sizeof(c));
	if(s.fail())
	{
		ERROR("Failed to read string size");
		return false;
	}
	uint size;
	if(byteOrder == BO_LITTLE_ENDIAN)
	{
		size = (c[0] | (c[1]<<8) | (c[2]<<16) | (c[3]<<24));
	}
	else
	{
		size = ((c[0]<<24) | (c[1]<<16) | (c[2]<< 8) | c[3]);
	}

	const uint buffSize = 1024;
	if((size + 1) > buffSize)
	{
		ERROR("String bugger than " << (buffSize + 1));
		return false;
	}
	char buff[buffSize];
	s.read(buff, size);
	if(s.fail())
	{
		ERROR("Failed to read " << size << " bytes");
		return false;
	}
	buff[size] = '\0';

	out =  buff;
	return true;
}


//======================================================================================================================
// readUintFromBinaryStream                                                                                            =
//======================================================================================================================
bool readUintFromBinaryStream(istream& s, uint& out, ByteOrder byteOrder)
{
	uchar c[4];
	s.read((char*)c, sizeof(c));
	if(s.fail())
	{
		ERROR("Failed to read uint");
		return false;
	}

	if(byteOrder == BO_LITTLE_ENDIAN)
	{
		out = (c[0] | (c[1]<<8) | (c[2]<<16) | (c[3]<<24));
	}
	else
	{
		out = ((c[0]<<24) | (c[1]<<16) | (c[2]<< 8) | c[3]);
	}
	return true;
}


//======================================================================================================================
// readFloatFromBinaryStream                                                                                           =
//======================================================================================================================
bool readFloatFromBinaryStream(istream& s, float& out)
{
	s.read((char*)&out, sizeof(float));
	if(s.fail())
	{
		ERROR("Failed to read float");
		return false;
	}
	return true;
}


} // end namesapce
