// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_BUFFER_COMMON_H
#define ANKI_GR_BUFFER_COMMON_H

#include "anki/gr/Common.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Buffer initializer.
class BufferInitializer
{
public:
	U32 m_size = 0;
	void* m_data = nullptr;
	BufferUsageBit m_usage = BufferUsageBit::UNIFORM_BUFFER;
};
/// @}

} // end namespace anki

#endif
