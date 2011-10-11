#include "anki/script/Common.h"
#include "anki/scene/MaterialRuntimeVariable.h"


WRAP(MaterialRuntimeVariable)
{
	class_<MaterialRuntimeVariable, noncopyable>("MaterialRuntimeVariable",
		no_init)
		.def("setValue", (void (MaterialRuntimeVariable::*)(const float&))
			(&MaterialRuntimeVariable::setValue))
		.def("setValue", (void (MaterialRuntimeVariable::*)(const Vec2&))
			(&MaterialRuntimeVariable::setValue))
		.def("setValue", (void (MaterialRuntimeVariable::*)(const Vec3&))
			(&MaterialRuntimeVariable::setValue))
	;
}
