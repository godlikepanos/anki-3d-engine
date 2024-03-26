// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Buffer.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Buffer implementation
class BufferImpl final : public Buffer
{
public:
	BufferImpl(CString name)
		: Buffer(name)
	{
	}

	~BufferImpl();

	Error init(const BufferInitInfo& inf);
};
/// @}

} // end namespace anki
