// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/D3D/D3DCommon.h>

namespace anki {

/// @addtogroup directx
/// @{

/// Buffer implementation
class BufferImpl final : public Buffer
{
	friend class Buffer;

public:
	BufferImpl(CString name)
		: Buffer(name)
	{
	}

	~BufferImpl();

	Error init(const BufferInitInfo& inf);

	ID3D12Resource& getD3DResource() const
	{
		return *m_resource;
	}

	D3D12_BUFFER_BARRIER computeBarrier(BufferUsageBit before, BufferUsageBit after) const;

private:
	ID3D12Resource* m_resource = nullptr;

#if ANKI_ASSERTIONS_ENABLED
	Bool m_mapped = false;
#endif

	D3D12_BARRIER_SYNC computeSync(BufferUsageBit usage) const;

	D3D12_BARRIER_ACCESS computeAccess(BufferUsageBit usage) const;
};
/// @}

} // end namespace anki
