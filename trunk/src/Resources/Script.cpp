#include "Script.h"
#include "Util.h"
#include "Util/Exception.h"


//==============================================================================
// load                                                                        =
//==============================================================================
void Script::load(const char* filename)
{
	source = Util::readFile(filename);
	if(source.length() < 1)
	{
		throw EXCEPTION("Cannot load script \"" + filename + "\"");
	}
}
