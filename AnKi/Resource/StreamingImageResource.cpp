// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/StreamingImageResource.h>
#include <AnKi/Resource/StreamingImageResourceManager.h>

namespace anki {

StreamingImageResource::~StreamingImageResource()
{
	StreamingImageResourceManager::getSingleton().freeImage(m_image);
}

Error StreamingImageResource::load(const ResourceFilename& filename, Bool async)
{
	return StreamingImageResourceManager::getSingleton().loadNewImage(filename, getUuid(), async, m_imageDescIdx, m_image);
}

Vec4 StreamingImageResource::getAverageColor() const
{
	return StreamingImageResourceManager::getSingleton().getImageAverageColor(m_image);
}

Bool StreamingImageResource::isLoaded() const
{
	return StreamingImageResourceManager::getSingleton().isImageLoaded(m_image);
}

Texture& StreamingImageResource::getTailChainTexture() const
{
	return StreamingImageResourceManager::getSingleton().getImageTailChainTexture(m_image);
}

} // end namespace anki
