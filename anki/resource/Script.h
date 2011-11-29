#ifndef ANKI_RESOURCE_SCRIPT_H
#define ANKI_RESOURCE_SCRIPT_H

#include <string>


namespace anki {


/// Python script resource
class Script
{
public:
	void load(const char* filename);

private:
	std::string source;
};


} // end namespace


#endif
