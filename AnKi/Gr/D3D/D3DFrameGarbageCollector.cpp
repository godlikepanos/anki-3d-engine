// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DFrameGarbageCollector.h>

namespace anki {

TextureGarbage::~TextureGarbage()
{
	for(U32 i = 0; i < m_descriptorHeapHandles.getSize(); ++i)
	{
		DescriptorFactory::getSingleton().freePersistent(m_descriptorHeapHandles[i]);
	}

	safeRelease(m_resource);
}

BufferGarbage::~BufferGarbage()
{
	safeRelease(m_resource);
}

} // end namespace anki
