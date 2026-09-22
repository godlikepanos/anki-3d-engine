// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>

namespace anki {

// Forward
class StreamingImage;

// Image resource class. It creates textures and then loads them in the GPU. It supports the formats of the ImageLoader.
class StreamingImageResource : public ResourceObject
{
	friend class StreamingImageResourceManager;

public:
	StreamingImageResource(CString fname, U32 uuid)
		: ResourceObject(fname, uuid, ResourceType::kStreamingImageResource)
	{
	}

	~StreamingImageResource();

	Error load(const ResourceFilename& filename, Bool async);

	Vec4 getAverageColor() const;

	// Returns true if the tail mip chain has been uploaded to the GPU
	Bool isLoaded() const;

	U32 getImageDescriptorIndex() const
	{
		ANKI_ASSERT(m_imageDescIdx < kMaxU32);
		return m_imageDescIdx;
	}

	Texture& getTailChainTexture() const;

private:
	StreamingImage* m_image = nullptr;

	U32 m_imageDescIdx = kMaxU32;
};

} // end namespace anki
