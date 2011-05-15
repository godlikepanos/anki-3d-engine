#include "ScriptingCommon.h"
#include "Pps.h"
#include "Hdr.h"
#include "Bl.h"


WRAP(Pps)
{
	class_<Pps, noncopyable>("Pps", no_init)
		.def("getHdr", &Pps::getHdr, return_value_policy<reference_existing_object>())
		.def("getBl", &Pps::getBl, return_value_policy<reference_existing_object>())
	;
}
