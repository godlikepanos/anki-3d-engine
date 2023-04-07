// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Ui/Font.h>
#include <AnKi/Ui/UiManager.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Buffer.h>
#include <AnKi/Gr/Texture.h>
#include <AnKi/Gr/CommandBuffer.h>

namespace anki {

Font::~Font()
{
	m_imFontAtlas.destroy();
}

Error Font::init(const CString& filename, ConstWeakArray<U32> fontHeights)
{
	m_imFontAtlas.init();

	// Load font in memory
	ResourceFilePtr file;
	ANKI_CHECK(ResourceManager::getSingleton().getFilesystem().openFile(filename, file));
	m_fontData.resize(U32(file->getSize()));
	ANKI_CHECK(file->read(&m_fontData[0], file->getSize()));

	m_fonts.resize(U32(fontHeights.getSize()));

	// Bake font
	ImFontConfig cfg;
	cfg.FontDataOwnedByAtlas = false;
	U32 count = 0;
	for(U32 height : fontHeights)
	{
		cfg.SizePixels = F32(height);

		m_fonts[count].m_imFont =
			m_imFontAtlas->AddFontFromMemoryTTF(&m_fontData[0], I32(m_fontData.getSize()), F32(height), &cfg);
		m_fonts[count].m_height = height;
		++count;
	}

	[[maybe_unused]] const Bool ok = m_imFontAtlas->Build();
	ANKI_ASSERT(ok);

	// Create the texture
	U8* img;
	int width, height;
	m_imFontAtlas->GetTexDataAsRGBA32(&img, &width, &height);
	createTexture(img, width, height);

	return Error::kNone;
}

void Font::createTexture(const void* data, U32 width, U32 height)
{
	ANKI_ASSERT(data && width > 0 && height > 0);

	// Create and populate the buffer
	const U32 buffSize = width * height * 4;
	BufferPtr buff = GrManager::getSingleton().newBuffer(
		BufferInitInfo(buffSize, BufferUsageBit::kTransferSource, BufferMapAccessBit::kWrite, "UI"));
	void* mapped = buff->map(0, buffSize, BufferMapAccessBit::kWrite);
	memcpy(mapped, data, buffSize);
	buff->flush(0, buffSize);
	buff->unmap();

	// Create the texture
	TextureInitInfo texInit("Font");
	texInit.m_width = width;
	texInit.m_height = height;
	texInit.m_format = Format::kR8G8B8A8_Unorm;
	texInit.m_usage =
		TextureUsageBit::kTransferDestination | TextureUsageBit::kSampledFragment | TextureUsageBit::kGenerateMipmaps;
	texInit.m_mipmapCount = 1; // No mips because it will appear blurry with trilinear filtering

	m_tex = GrManager::getSingleton().newTexture(texInit);

	// Create the whole texture view
	m_texView = GrManager::getSingleton().newTextureView(TextureViewInitInfo(m_tex, "Font"));
	m_imFontAtlas->SetTexID(UiImageId(m_texView));

	// Do the copy
	constexpr TextureSurfaceInfo surf(0, 0, 0, 0);
	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

	TextureViewInitInfo viewInit(m_tex, surf, DepthStencilAspectBit::kNone, "TempFont");
	TextureViewPtr tmpView = GrManager::getSingleton().newTextureView(viewInit);

	TextureBarrierInfo barrier = {m_tex.get(), TextureUsageBit::kNone, TextureUsageBit::kTransferDestination, surf};
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

	cmdb->copyBufferToTextureView(buff, 0, buffSize, tmpView);

	barrier.m_previousUsage = TextureUsageBit::kTransferDestination;
	barrier.m_nextUsage = TextureUsageBit::kGenerateMipmaps;
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

	// Gen mips
	cmdb->generateMipmaps2d(m_texView);

	barrier.m_previousUsage = TextureUsageBit::kGenerateMipmaps;
	barrier.m_nextUsage = TextureUsageBit::kSampledFragment;
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

	cmdb->flush();
}

} // end namespace anki
