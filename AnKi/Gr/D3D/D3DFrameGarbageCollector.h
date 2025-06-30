// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/D3D/D3DDescriptor.h>
#include <AnKi/Gr/BackendCommon/FrameGarbageCollector.h>

namespace anki {

/// @addtogroup directx
/// @{

/// @memberof FrameGarbageCollector
class TextureGarbage : public IntrusiveListEnabled<TextureGarbage>
{
public:
	GrDynamicArray<DescriptorHeapHandle> m_descriptorHeapHandles;
	ID3D12Resource* m_resource = nullptr;

	~TextureGarbage();
};

/// @memberof FrameGarbageCollector
class BufferGarbage : public IntrusiveListEnabled<BufferGarbage>
{
public:
	ID3D12Resource* m_resource = nullptr;

	~BufferGarbage();
};

/// @memberof FrameGarbageCollector
class ASGarbage : public IntrusiveListEnabled<ASGarbage>
{
};

class D3DFrameGarbageCollector :
	public FrameGarbageCollector<TextureGarbage, BufferGarbage, ASGarbage>,
	public MakeSingleton<D3DFrameGarbageCollector>
{
};
/// @}

} // end namespace anki
