#include "ScriptingCommon.h"
#include "r/Pps.h"
#include "r/Hdr.h"
#include "r/Bl.h"


WRAP(Pps)
{
	using namespace r;

	class_<Pps, noncopyable>("r_Pps", no_init)
		.def("getHdr", (Hdr& (Pps::*)())(&Pps::getHdr),
			return_value_policy<reference_existing_object>())

		.def("getBl", (Bl& (Pps::*)())(&Pps::getBl),
			return_value_policy<reference_existing_object>())
	;
}
