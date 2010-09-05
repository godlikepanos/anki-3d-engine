#include "Script.h"
#include "Util.h"


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool Script::load(const char* filename)
{
	source = Util::readFile(filename);
	if(source.length() < 1)
	{
		ERROR("Cannot load script \"" << filename << "\"");
		return false;
	}
	return true;
}
