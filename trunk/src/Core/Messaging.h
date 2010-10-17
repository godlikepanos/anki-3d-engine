#ifndef MESSAGING_H
#define MESSAGING_H

#include "App.h"
#include "MessageHandler.h"

#define INFO(x) \
	app->getMessageHandler().write(std::string("Info: ") + x)

#define WARNING(x) \
	app->getMessageHandler().write(std::string("Warning: ") + x)


#endif
