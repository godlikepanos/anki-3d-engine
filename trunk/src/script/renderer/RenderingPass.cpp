#include "anki/script/Common.h"
#include "anki/renderer/RenderingPass.h"

ANKI_WRAP(SwitchableRenderingPass)
{
	class_<SwitchableRenderingPass, noncopyable>("SwitchableRenderingPass",
		no_init)
		.def("getEnabled", (bool (SwitchableRenderingPass::*)() const)(
			&SwitchableRenderingPass::getEnabled))
		.def("setEnabled", &SwitchableRenderingPass::setEnabled)
	;
}
