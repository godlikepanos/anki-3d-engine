#include "Script.h"
#include "util/Util.h"
#include "util/Exception.h"


//==============================================================================
// load                                                                        =
//==============================================================================
void Script::load(const char* filename)
{
	source = util::readFile(filename);
	if(source.length() < 1)
	{
		throw EXCEPTION("Cannot load script \"" + filename + "\"");
	}
}
