#ifndef SCRIPT_H
#define SCRIPT_H

#include "Common.h"
#include "Resource.h"


/**
 * Python script resource
 */
class Script: public Resource
{
	public:
		Script();
		~Script() {}

		bool load(const char* filename);

	private:
		string source;
};


inline Script::Script():
	Resource(RT_SCRIPT)
{}


#endif
