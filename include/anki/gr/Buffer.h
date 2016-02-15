// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// GPU buffer.
class Buffer : public GrObject
{
public:
	/// Construct.
	Buffer(GrManager* manager);

	/// Destroy.
	~Buffer();

	/// Access the implementation.
	BufferImpl& getImplementation()
	{
		return *m_impl;
	}

	/// Allocate the buffer.
	void init(PtrSize size, BufferUsageBit usage, BufferAccessBit access);

	/// Map the buffer.
	void* map(PtrSize offset, PtrSize range, BufferAccessBit access);

	/// Unmap the buffer.
	void unmap();

private:
	UniquePtr<BufferImpl> m_impl;
};
/// @}

} // end namespace anki
