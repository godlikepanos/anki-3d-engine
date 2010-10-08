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
		THROW_EXCEPTION("Cannot load script \"" << filename << "\"");
	}
	return true;
}
