#include "anki/resource/Script.h"
#include "anki/util/Util.h"
#include "anki/util/Exception.h"


namespace anki {


//==============================================================================
// load                                                                        =
//==============================================================================
void Script::load(const char* filename)
{
	source = Util::readFile(filename);
	if(source.length() < 1)
	{
		throw ANKI_EXCEPTION("Cannot load script \"" + filename + "\"");
	}
}


} // end namespace
