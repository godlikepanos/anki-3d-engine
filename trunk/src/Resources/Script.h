#ifndef SCRIPT_H
#define SCRIPT_H

#include "Common.h"
#include "Resource.h"


/// Python script resource
class Script: public Resource
{
	public:
		Script(): Resource(RT_SCRIPT) {}
		~Script() {}
		void load(const char* filename);

	private:
		string source;
};


#endif
