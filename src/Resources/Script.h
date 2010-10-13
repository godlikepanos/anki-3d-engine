#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include "Resource.h"


/// Python script resource
class Script: public Resource
{
	public:
		Script(): Resource(RT_SCRIPT) {}
		~Script() {}
		void load(const char* filename);

	private:
		std::string source;
};


#endif
