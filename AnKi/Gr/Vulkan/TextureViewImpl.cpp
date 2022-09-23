// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/TextureViewImpl.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

namespace anki {

TextureViewImpl::~TextureViewImpl()
{
}

Error TextureViewImpl::init(const TextureViewInitInfo& inf)
{
	ANKI_ASSERT(inf.isValid());

	// Store some stuff
	m_subresource = inf;

	m_tex = inf.m_texture;
	const TextureImpl& tex = static_cast<const TextureImpl&>(*m_tex);
	ANKI_ASSERT(tex.isSubresourceValid(inf));

	// Ask the texture for a view
	m_microImageView = &tex.getOrCreateView(inf);
	m_handle = m_microImageView->getHandle();
	m_texType = m_microImageView->getDerivedTextureType();

	// Create the hash
	Array<U64, 2> toHash = {tex.getUuid(), ptrToNumber(m_handle)};
	m_hash = computeHash(&toHash[0], sizeof(toHash));

	return Error::kNone;
}

U32 TextureViewImpl::getOrCreateBindlessIndex()
{
	if(m_bindlessIndex == kMaxU32)
	{
		m_bindlessIndex = m_microImageView->getOrCreateBindlessIndex(getGrManagerImpl());
	}

	return m_bindlessIndex;
}

} // end namespace anki
