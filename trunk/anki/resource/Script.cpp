#include "anki/resource/Script.h"
#include "anki/util/Util.h"
#include "anki/util/Exception.h"


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
