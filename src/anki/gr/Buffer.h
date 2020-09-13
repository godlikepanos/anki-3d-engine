// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Buffer init info.
/// @memberof Buffer
class BufferInitInfo : public GrBaseInitInfo
{
public:
	PtrSize m_size = 0;
	BufferUsageBit m_usage = BufferUsageBit::NONE;
	BufferMapAccessBit m_mapAccess = BufferMapAccessBit::NONE;

	BufferInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	BufferInitInfo(PtrSize size, BufferUsageBit usage, BufferMapAccessBit mapAccess, CString name = {})
		: GrBaseInitInfo(name)
		, m_size(size)
		, m_usage(usage)
		, m_mapAccess(mapAccess)
	{
	}

	Bool isValid() const
	{
		return m_size && !!m_usage;
	}
};

/// GPU buffer.
class Buffer : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::BUFFER;

	/// Return the size of the buffer.
	PtrSize getSize() const
	{
		ANKI_ASSERT(m_size > 0);
		return m_size;
	}

	/// Return the BufferUsageBit of the Buffer.
	BufferUsageBit getBufferUsage() const
	{
		ANKI_ASSERT(!!m_usage);
		return m_usage;
	}

	/// Return the BufferMapAccessBit of the Buffer.
	BufferMapAccessBit getMapAccess() const
	{
		return m_access;
	}

	/// Map the buffer.
	/// @param offset The starting offset.
	/// @param range The range to map or MAX_PTR_SIZE to map until the end.
	/// @param access The access to the buffer.
	void* map(PtrSize offset, PtrSize range, BufferMapAccessBit access);

	/// Convenience map method.
	/// @param offset The starting offset.
	/// @param elementCount The number of T element sto map.
	/// @param access The access to the buffer.
	/// @return The array that was mapped.
	template<typename T>
	WeakArray<T, PtrSize> map(PtrSize offset, PtrSize elementCount, BufferMapAccessBit access)
	{
		return WeakArray<T, PtrSize>(static_cast<T*>(map(offset, sizeof(T) * elementCount, access)), elementCount);
	}

	/// Unmap the buffer.
	void unmap();

	/// Get the GPU adress of the buffer.
	U64 getGpuAddress() const
	{
		ANKI_ASSERT(m_gpuAddress);
		return m_gpuAddress;
	}

protected:
	PtrSize m_size = 0;
	BufferUsageBit m_usage = BufferUsageBit::NONE;
	BufferMapAccessBit m_access = BufferMapAccessBit::NONE;
	U64 m_gpuAddress = 0;

	/// Construct.
	Buffer(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
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
