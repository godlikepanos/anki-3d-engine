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
// cutPath                                                                                                             =
//======================================================================================================================
char* cutPath(const char* path)
{
	char* str = (char*)path + strlen(path) - 1;
	for(;;)
	{
		if((str-path)<=-1 || *str=='/' || *str=='\\')  break;
		str--;
	}
	return str+1;
}


//======================================================================================================================
// getPath                                                                                                             =
//======================================================================================================================
string getPath(const char* path)
{
	char* str = (char*)path + strlen(path) - 1;
	for(;;)
	{
		if((str-path)<=-1 || *str=='/' || *str=='\\')  break;
		-- str;
	}
	++str;
	int n = str - path;
	DEBUG_ERR(n<0 || n>100); // check the func. probably something wrong
	string retStr;
	retStr.assign(path, n);
	return retStr;
}


//======================================================================================================================
// getFunctionFromPrettyFunction                                                                                       =
//======================================================================================================================
string getFunctionFromPrettyFunction(const char* prettyFunction)
{
	string ret(prettyFunction);

	size_t index = ret.find("(");

	if (index != string::npos)
		ret.erase(index);

	index = ret.rfind(" ");
	if (index != string::npos)
		ret.erase(0, index + 1);

	return ret;
}


//======================================================================================================================
// getFileExtension                                                                                                    =
//======================================================================================================================
string getFileExtension(const char* path)
{
	char* str = (char*)path + strlen(path) - 1;
	for(;;)
	{
		if((str-path)<=-1 || *str=='.')  break;
		str--;
	}

	if(str == (char*)path + strlen(path) - 1) ERROR("Please put something after the '.' in path \"" << path << "\". Idiot");
	if(str+1 == path) ERROR("Path \"" << path << "\" doesnt contain a '.'. What the fuck?");

	return str+1;
}


//======================================================================================================================
// intToStr                                                                                                            =
//======================================================================================================================
string intToStr(int i)
{
	char str [256];
	char* pstr = str + ((sizeof(str)/sizeof(char)) - 1);
	bool negative = false;

	*pstr = '\0';

	if(i < 0)
	{
		i = -i;
		negative = true;
	}

	do
	{
		--pstr;
		int remain = i % 10;
		i = i / 10;
		*pstr = '0' + remain;
	} while(i != 0);

	if(negative)
	{
		--pstr;
		*pstr = '-';
	}

	return string(pstr);
}


//======================================================================================================================
// floatToStr                                                                                                          =
//======================================================================================================================
string floatToStr(float f)
{
	char tmp [128];

	sprintf(tmp, "%f", f);

	string s(tmp);
	return s;
}


} // end namesapce
