// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_CORE_CONFIG_H
#define ANKI_CORE_CONFIG_H

#include "anki/misc/ConfigSet.h"

namespace anki {

/// @addtogroup core
/// @{

/// Global configuration set
class Config: public ConfigSet
{
public:
	Config();
	~Config();
};

/// @}

} // end namespace anki

#endif


