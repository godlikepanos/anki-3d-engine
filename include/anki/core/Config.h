// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

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

