#include "anki/script/Common.h"
#include "anki/renderer/Pps.h"
#include "anki/renderer/Hdr.h"
#include "anki/renderer/Bl.h"

ANKI_WRAP(Pps)
{
	class_<Pps, noncopyable>("Pps", no_init)
		.def("getHdr", (Hdr& (Pps::*)())(&Pps::getHdr),
			return_value_policy<reference_existing_object>())

		.def("getBl", (Bl& (Pps::*)())(&Pps::getBl),
			return_value_policy<reference_existing_object>())
	;
}
