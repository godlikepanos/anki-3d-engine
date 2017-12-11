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

/// Buffer init info.
class BufferInitInfo : public GrBaseInitInfo
{
public:
	PtrSize m_size = 0;
	BufferUsageBit m_usage = BufferUsageBit::NONE;
	BufferMapAccessBit m_access = BufferMapAccessBit::NONE;

	BufferInitInfo() = default;

	BufferInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	BufferInitInfo(PtrSize size, BufferUsageBit usage, BufferMapAccessBit access, CString name = {})
		: GrBaseInitInfo(name)
		, m_size(size)
		, m_usage(usage)
		, m_access(access)
	{
	}
};

/// GPU buffer.
class Buffer : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::BUFFER;

	/// Map the buffer.
	void* map(PtrSize offset, PtrSize range, BufferMapAccessBit access);

	/// Unmap the buffer.
	void unmap();

protected:
	/// Construct.
	Buffer(GrManager* manager)
		: GrObject(manager, CLASS_TYPE)
	{
	}

	/// Destroy.
	~Buffer()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT Buffer* newInstance(GrManager* manager, const BufferInitInfo& init);
};
/// @}

} // end namespace anki
