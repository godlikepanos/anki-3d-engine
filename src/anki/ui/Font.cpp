// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/Font.h>
#include <anki/ui/UiManager.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/Buffer.h>
#include <anki/gr/Texture.h>
#include <anki/gr/CommandBuffer.h>

namespace anki
{

Font::~Font()
{
	nk_font_atlas_clear(&m_atlas);
	m_fonts.destroy(getAllocator());
}

Error Font::init(const CString& filename, const std::initializer_list<U32>& fontHeights)
{
	// Load font in memory
	ResourceFilePtr file;
	ANKI_CHECK(m_manager->getResourceManager().getFilesystem().openFile(filename, file));
	DynamicArrayAuto<U8> fontData(getAllocator());
	fontData.create(file->getSize());
	ANKI_CHECK(file->read(&fontData[0], file->getSize()));

	m_fonts.create(getAllocator(), fontHeights.size());

	// Bake font
	nk_allocator nkAlloc = makeNkAllocator(&getAllocator().getMemoryPool());
	nk_font_atlas_init_custom(&m_atlas, &nkAlloc, &nkAlloc);
	nk_font_atlas_begin(&m_atlas);

	U count = 0;
	for(U32 height : fontHeights)
	{
		struct nk_font_config cfg = nk_font_config(height);
		cfg.oversample_h = 4;
		cfg.oversample_v = 4;
		m_fonts[count].m_font = nk_font_atlas_add_from_memory(&m_atlas, &fontData[0], fontData.getSize(), height, &cfg);
		m_fonts[count].m_height = height;
		++count;
	}

	int width, height;
	const void* img = nk_font_atlas_bake(&m_atlas, &width, &height, NK_FONT_ATLAS_RGBA32);

	// Create the texture
	createTexture(img, width, height);

	// End building
	nk_handle texHandle;
	texHandle.ptr = numberToPtr<void*>(ptrToNumber(m_texView.get()) | FONT_TEXTURE_MASK);
	nk_font_atlas_end(&m_atlas, texHandle, nullptr);

	nk_font_atlas_cleanup(&m_atlas);

	return Error::NONE;
}

void Font::createTexture(const void* data, U32 width, U32 height)
{
	ANKI_ASSERT(data && width > 0 && height > 0);

	// Create and populate the buffer
	PtrSize buffSize = width * height * 4;
	BufferPtr buff = m_manager->getGrManager().newBuffer(
		BufferInitInfo(buffSize, BufferUsageBit::BUFFER_UPLOAD_SOURCE, BufferMapAccessBit::WRITE, "UI"));
	void* mapped = buff->map(0, buffSize, BufferMapAccessBit::WRITE);
	memcpy(mapped, data, buffSize);
	buff->unmap();

	// Create the texture
	TextureInitInfo texInit("Font");
	texInit.m_width = width;
	texInit.m_height = height;
	texInit.m_format = Format::R8G8B8A8_UNORM;
	texInit.m_usage =
		TextureUsageBit::TRANSFER_DESTINATION | TextureUsageBit::SAMPLED_FRAGMENT | TextureUsageBit::GENERATE_MIPMAPS;
	texInit.m_mipmapCount = 4;

	m_tex = m_manager->getGrManager().newTexture(texInit);

	// Create the whole texture view
	m_texView = m_manager->getGrManager().newTextureView(TextureViewInitInfo(m_tex, "Font"));

	// Do the copy
	static const TextureSurfaceInfo surf(0, 0, 0, 0);
	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags = CommandBufferFlag::TRANSFER_WORK | CommandBufferFlag::SMALL_BATCH;
	CommandBufferPtr cmdb = m_manager->getGrManager().newCommandBuffer(cmdbInit);
	{
		TextureViewInitInfo viewInit(m_tex, surf, DepthStencilAspectBit::NONE, "TempFont");
		TextureViewPtr tmpView = m_manager->getGrManager().newTextureView(viewInit);

		cmdb->setTextureSurfaceBarrier(m_tex, TextureUsageBit::NONE, TextureUsageBit::TRANSFER_DESTINATION, surf);
		cmdb->copyBufferToTextureView(buff, 0, buffSize, tmpView);
		cmdb->setTextureSurfaceBarrier(
			m_tex, TextureUsageBit::TRANSFER_DESTINATION, TextureUsageBit::GENERATE_MIPMAPS, surf);
	}

	// Gen mips
	cmdb->generateMipmaps2d(m_texView);
	cmdb->setTextureSurfaceBarrier(m_tex, TextureUsageBit::GENERATE_MIPMAPS, TextureUsageBit::SAMPLED_FRAGMENT, surf);

	cmdb->flush();
}

} // end namespace anki
