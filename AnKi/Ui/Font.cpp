// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Gr/ShaderProgram.h>

namespace anki {

Font::~Font()
{
	m_imFontAtlas.destroy();
}

Error Font::init(const CString& filename, ConstWeakArray<U32> fontHeights)
{
	m_imFontAtlas.construct();

	// Load font in memory
	ResourceFilePtr file;
	ANKI_CHECK(ResourceFilesystem::getSingleton().openFile(filename, file));
	m_fontData.resize(U32(file->getSize()));
	ANKI_CHECK(file->read(&m_fontData[0], file->getSize()));

	m_fonts.resize(fontHeights.getSize());

	// Bake font
	ImFontConfig cfg;
	cfg.FontDataOwnedByAtlas = false;
	U32 count = 0;
	for(U32 height : fontHeights)
	{
		height = U32(F32(height) * g_cvarUiGlobalFontScale) + g_cvarUiGlobalFontBias;
		cfg.SizePixels = F32(height);

		m_fonts[count].m_imFont = m_imFontAtlas->AddFontFromMemoryTTF(&m_fontData[0], I32(m_fontData.getSize()), F32(height), &cfg);
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
	BufferPtr buff = GrManager::getSingleton().newBuffer(BufferInitInfo(buffSize, BufferUsageBit::kCopySource, BufferMapAccessBit::kWrite, "UI"));
	void* mapped = buff->map(0, buffSize, BufferMapAccessBit::kWrite);
	memcpy(mapped, data, buffSize);
	buff->flush(0, buffSize);
	buff->unmap();

	// Create the texture
	TextureInitInfo texInit("Font");
	texInit.m_width = width;
	texInit.m_height = height;
	texInit.m_format = Format::kR8G8B8A8_Unorm;
	texInit.m_usage = TextureUsageBit::kCopyDestination | TextureUsageBit::kSrvPixel;
	texInit.m_mipmapCount = 1; // No mips because it will appear blurry with trilinear filtering

	m_tex = GrManager::getSingleton().newTexture(texInit);

	// Create the whole texture view
	m_imgData.m_textureView = TextureView(m_tex.get(), TextureSubresourceDesc::all());
	m_imFontAtlas->SetTexID(UiImageId(&m_imgData));

	// Do the copy
	const TextureView firstMipView(m_tex.get(), TextureSubresourceDesc::firstSurface());

	CommandBufferInitInfo cmdbInit;
	cmdbInit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
	CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(cmdbInit);

	TextureBarrierInfo barrier = {firstMipView, TextureUsageBit::kNone, TextureUsageBit::kCopyDestination};
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

	cmdb->copyBufferToTexture(BufferView(buff.get()), firstMipView);

	barrier.m_previousUsage = TextureUsageBit::kCopyDestination;
	barrier.m_nextUsage = TextureUsageBit::kSrvPixel;
	cmdb->setPipelineBarrier({&barrier, 1}, {}, {});

	cmdb->endRecording();
	GrManager::getSingleton().submit(cmdb.get());
}

} // end namespace anki
