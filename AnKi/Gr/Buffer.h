// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup graphics
/// @{

/// Buffer init info.
/// @memberof Buffer
class BufferInitInfo : public GrBaseInitInfo
{
public:
	PtrSize m_size = 0;
	BufferUsageBit m_usage = BufferUsageBit::kNone;
	BufferMapAccessBit m_mapAccess = BufferMapAccessBit::kNone;

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
	static constexpr GrObjectType kClassType = GrObjectType::kBuffer;

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
	/// @param range The range to map or kMaxPtrSize to map until the end.
	/// @param access The access to the buffer.
	void* map(PtrSize offset, PtrSize range, BufferMapAccessBit access);

	/// Flush the buffer from the CPU caches. Call it to make the buffer memory available to the GPU.
	/// @param offset The starting offset.
	/// @param range The range to map or kMaxPtrSize to map until the end.
	void flush(PtrSize offset, PtrSize range) const;

	/// Invalidate the buffer from the CPU caches. Call it to ready the buffer to see GPU updates.
	/// @param offset The starting offset.
	/// @param range The range to map or kMaxPtrSize to map until the end.
	void invalidate(PtrSize offset, PtrSize range) const;

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
	BufferUsageBit m_usage = BufferUsageBit::kNone;
	BufferMapAccessBit m_access = BufferMapAccessBit::kNone;
	U64 m_gpuAddress = 0;

	/// Construct.
	Buffer(CString name)
		: GrObject(kClassType, name)
	{
	}

	/// Destroy.
	~Buffer()
	{
	}

private:
	/// Allocate and initialize a new instance.
	[[nodiscard]] static Buffer* newInstance(const BufferInitInfo& init);
};

/// A part of a buffer.
class BufferView
{
public:
	BufferView() = default;

	BufferView(const BufferView&) = default;

	explicit BufferView(Buffer* buffer)
		: m_buffer(buffer)
		, m_offset(0)
		, m_range(buffer->getSize())
	{
		validate();
	}

	BufferView(Buffer* buffer, PtrSize offset, PtrSize range)
		: m_buffer(buffer)
		, m_offset(offset)
		, m_range(range)
	{
		validate();
	}

	BufferView& operator=(const BufferView&) = default;

	explicit operator Bool() const
	{
		return isValid();
	}

	[[nodiscard]] Buffer& getBuffer() const
	{
		validate();
		return *m_buffer;
	}

	[[nodiscard]] const PtrSize& getOffset() const
	{
		validate();
		return m_offset;
	}

	BufferView& setOffset(PtrSize offset)
	{
		validate();
		m_offset = offset;
		validate();
		return *this;
	}

	BufferView& incrementOffset(PtrSize bytes)
	{
		validate();
		ANKI_ASSERT(m_range >= bytes);
		m_range -= bytes;
		m_offset += bytes;
		if(m_range == 0)
		{
			*this = {};
		}
		else
		{
			validate();
		}
		return *this;
	}

	[[nodiscard]] const PtrSize& getRange() const
	{
		validate();
		return m_range;
	}

	BufferView& setRange(PtrSize range)
	{
		validate();
		if(range != 0)
		{
			m_range = range;
			validate();
		}
		else
		{
			*this = {};
		}
		return *this;
	}

	[[nodiscard]] Bool isValid() const
	{
		return m_buffer != nullptr;
	}

	[[nodiscard]] Bool overlaps(const BufferView& b) const
	{
		validate();
		b.validate();
		Bool overlaps = m_buffer == b.m_buffer;
		if(m_offset <= b.m_offset)
		{
			overlaps = overlaps && (m_offset + m_range > b.m_offset);
		}
		else
		{
			overlaps = overlaps && (b.m_offset + b.m_range > m_offset);
		}

		return overlaps;
	}

private:
	Buffer* m_buffer = nullptr;
	PtrSize m_offset = kMaxPtrSize;
	PtrSize m_range = 0;

	void validate() const
	{
		ANKI_ASSERT(m_buffer && m_range > 0);
		ANKI_ASSERT(m_range <= m_buffer->getSize() && m_offset < m_buffer->getSize()); // Do that to ensure the next line won't overflow
		ANKI_ASSERT(m_offset + m_range <= m_buffer->getSize());
	}
};
/// @}

} // end namespace anki
