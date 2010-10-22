#ifndef MESSAGING_H
#define MESSAGING_H

#include <string>
#include "App.h"
#include "MessageHandler.h"


#define MESSAGE(x) \
	app->getMessageHandler().write(__FILE__, __LINE__, __func__, std::string() + x)

#define INFO(x) MESSAGE(std::string("Info: ") + x)

#define WARNING(x) MESSAGE(std::string("Warning: ") + x)


#endif
