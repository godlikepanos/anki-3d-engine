#include "ScriptingCommon.h"
#include "Renderer/Pps.h"
#include "Renderer/Hdr.h"
#include "Renderer/Bl.h"


WRAP(Pps)
{
	class_<R::Pps, noncopyable>("Pps", no_init)
		.def("getHdr", (R::Hdr& (R::Pps::*)())(&R::Pps::getHdr),
			return_value_policy<reference_existing_object>())

		.def("getBl", (R::Bl& (R::Pps::*)())(&R::Pps::getBl),
			return_value_policy<reference_existing_object>())
	;
}
