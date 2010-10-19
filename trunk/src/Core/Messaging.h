#ifndef MESSAGING_H
#define MESSAGING_H

#include <string>
#include "App.h"
#include "MessageHandler.h"


#define INFO(x) \
	app->getMessageHandler().write(__FILE__, __LINE__, __func__, std::string("Info: ") + x)

#define WARNING(x) \
	app->getMessageHandler().write(__FILE__, __LINE__, __func__, std::string("Warning: ") + x)


#endif
