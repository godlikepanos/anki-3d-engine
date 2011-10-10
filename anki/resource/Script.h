#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>


/// Python script resource
class Script
{
	public:
		void load(const char* filename);

	private:
		std::string source;
};


#endif
