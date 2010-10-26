#include "ScriptingCommon.h"
#include "MessageHandler.h"


WRAP(MessageHandler)
{
	class_<MessageHandler, noncopyable>("MessageHandler", no_init)
		.def("write", &MessageHandler::write)
	;
}
