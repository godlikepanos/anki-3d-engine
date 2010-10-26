#include "ScriptingCommon.h"
#include "Pps.h"
#include "Hdr.h"


WRAP(Pps)
{
	class_<Pps, noncopyable>("Pps", no_init)
		.def_readonly("hdr", &Pps::hdr)
	;
}
