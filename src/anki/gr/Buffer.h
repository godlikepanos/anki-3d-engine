// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class Buffer final : public GrObject
{
	ANKI_GR_OBJECT

public:
	/// Map the buffer.
	void* map(PtrSize offset, PtrSize range, BufferMapAccessBit access);

	/// Unmap the buffer.
	void unmap();

anki_internal:
	static const GrObjectType CLASS_TYPE = GrObjectType::BUFFER;

	UniquePtr<BufferImpl> m_impl;

	/// Construct.
	Buffer(GrManager* manager, U64 hash, GrObjectCache* cache);

	/// Destroy.
	~Buffer();

	/// Allocate the buffer.
	void init(PtrSize size, BufferUsageBit usage, BufferMapAccessBit access);
};
/// @}

} // end namespace anki
