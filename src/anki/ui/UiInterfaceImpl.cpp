// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/UiInterfaceImpl.h>
#include <anki/resource/ResourceManager.h>
#include <anki/resource/GenericResource.h>

namespace anki
{

struct Vertex
{
	Vec2 m_pos;
	Vec2 m_uv;
	Array<U8, 4> m_color;
};

UiInterfaceImpl::UiInterfaceImpl(UiAllocator alloc)
	: UiInterface(alloc)
{
}

UiInterfaceImpl::~UiInterfaceImpl()
{
}

Error UiInterfaceImpl::init(GrManager* gr, ResourceManager* rc)
{
	// Load shaders
	ANKI_CHECK(rc->loadResource("shaders/UiLines.vert.glsl", m_stages[StageId::LINES].m_vShader));

	ANKI_CHECK(rc->loadResource("shaders/UiLines.frag.glsl", m_stages[StageId::LINES].m_fShader));

// Init pplines
#if 0
	PipelineInitInfo ppinit;
	ppinit.m_vertex.m_bindingCount = 1;
	ppinit.m_vertex.m_bindings[0].m_stride = sizeof(Vertex);
	ppinit.m_vertex.m_attributeCount = 3;
	ppinit.m_vertex.m_attributes[0].m_format = PixelFormat(ComponentFormat::R32G32, TransformFormat::FLOAT);
	ppinit.m_vertex.m_attributes[0].m_offset = 0;
	ppinit.m_vertex.m_attributes[1].m_format = PixelFormat(ComponentFormat::R32G32, TransformFormat::FLOAT);
	ppinit.m_vertex.m_attributes[1].m_offset = sizeof(Vec2);
	ppinit.m_vertex.m_attributes[2].m_format = PixelFormat(ComponentFormat::R8G8B8A8, TransformFormat::UNORM);
	ppinit.m_vertex.m_attributes[2].m_offset = sizeof(Vec2) * 2;

	ppinit.m_inputAssembler.m_topology = PrimitiveTopology::LINES;

	ppinit.m_depthStencil.m_depthWriteEnabled = false;
	ppinit.m_depthStencil.m_depthCompareFunction = CompareOperation::ALWAYS;

	ppinit.m_color.m_attachmentCount = 1;
	ppinit.m_color.m_attachments[0].m_format = PixelFormat(ComponentFormat::R8G8B8, TransformFormat::UNORM);
	ppinit.m_color.m_attachments[0].m_srcBlendFactor = BlendFactor::SRC_ALPHA;
	ppinit.m_color.m_attachments[0].m_dstBlendFactor = BlendFactor::ONE_MINUS_SRC_ALPHA;

	ppinit.m_shaders[U(ShaderType::VERTEX)] = m_stages[StageId::LINES].m_vShader->getGrShader();
	ppinit.m_shaders[U(ShaderType::FRAGMENT)] = m_stages[StageId::LINES].m_fShader->getGrShader();
	m_stages[StageId::LINES].m_ppline = gr->newInstance<Pipeline>(ppinit);
#endif

	// Init buffers
	for(U s = 0; s < StageId::COUNT; ++s)
	{
		for(U i = 0; i < m_stages[s].m_vertBuffs.getSize(); ++i)
		{
			m_stages[s].m_vertBuffs[i] =
				gr->newInstance<Buffer>(MAX_VERTS * sizeof(Vertex), BufferUsageBit::VERTEX, BufferMapAccessBit::WRITE);
		}
	}

// Init resource groups
#if 0
	for(U s = 0; s < StageId::COUNT; ++s)
	{
		for(U i = 0; i < m_stages[s].m_rcGroups.getSize(); ++i)
		{
			ResourceGroupInitInfo rcinit;
			rcinit.m_vertexBuffers[0].m_buffer = m_stages[s].m_vertBuffs[i];

			if(s == StageId::TEXTURED_TRIANGLES)
			{
				// TODO
			}

			m_stages[s].m_rcGroups[i] = gr->newInstance<ResourceGroup>(rcinit);
		}
	}
#endif

	return ErrorCode::NONE;
}

void UiInterfaceImpl::beginRendering(CommandBufferPtr cmdb)
{
	m_cmdb = cmdb;

	// Map buffers
	for(U s = 0; s < StageId::COUNT; ++s)
	{
		BufferPtr buff = m_stages[s].m_vertBuffs[m_timestamp];

		m_vertMappings[s] = static_cast<Vertex*>(buff->map(0, sizeof(Vertex) * MAX_VERTS, BufferMapAccessBit::WRITE));

		m_vertCounts[s] = 0;
	}
}

void UiInterfaceImpl::endRendering()
{
	m_cmdb.reset(nullptr);

	for(U s = 0; s < StageId::COUNT; ++s)
	{
		BufferPtr buff = m_stages[s].m_vertBuffs[m_timestamp];
		buff->unmap();
	}

	m_timestamp = (m_timestamp + 1) % MAX_FRAMES_IN_FLIGHT;
}

void UiInterfaceImpl::drawLines(const WeakArray<UVec2>& positions, const Color& color, const UVec2& canvasSize)
{
	StageId stageId = StageId::LINES;

	ANKI_ASSERT(m_vertCounts[stageId] + positions.getSize() <= MAX_VERTS);

	// m_cmdb->bindPipeline(m_stages[StageId::LINES].m_ppline);
	// m_cmdb->bindResourceGroup(m_stages[StageId::LINES].m_rcGroups[m_timestamp], 0, nullptr);
	// m_cmdb->drawArrays(positions.getSize(), 1, m_vertCounts[stageId]);

	for(const UVec2& pos : positions)
	{
		Vertex v;
		v.m_pos = Vec2(pos.x(), pos.y()) / Vec2(canvasSize.x(), canvasSize.y()) * 2.0 - 1.0;
		v.m_uv = Vec2(0.0);
		Color c = color * 255.0;
		v.m_color = {{U8(c[0]), U8(c[1]), U8(c[2]), U8(c[3])}};

		m_vertMappings[stageId][m_vertCounts[stageId]] = v;

		++m_vertCounts[stageId];
	}
}

void UiInterfaceImpl::drawImage(UiImagePtr image, const Rect& uvs, const Rect& drawingRect, const UVec2& canvasSize)
{
	StageId stageId = StageId::TEXTURED_TRIANGLES;

	ANKI_ASSERT(m_vertCounts[stageId] + 4 <= MAX_VERTS);
}

Error UiInterfaceImpl::loadImage(const CString& filename, IntrusivePtr<UiImage>& img)
{
	TextureResourcePtr texture;
	ANKI_CHECK(m_rc->loadResource(filename, texture));
	UiImageImpl* impl = getAllocator().newInstance<UiImageImpl>(this);
	impl->m_resource = texture;
	impl->m_texture = texture->getGrTexture();

	img.reset(impl);

	return ErrorCode::NONE;
}

Error UiInterfaceImpl::createR8Image(const WeakArray<U8>& data, const UVec2& size, IntrusivePtr<UiImage>& img)
{
#if 0
	ANKI_ASSERT(data.getSize() == size.x() * size.y());

	// Calc mip count
	U s = min(size.x(), size.y());
	U mipCount = 0;
	while(s > 0)
	{
		++mipCount;
		s /= 2;
	}

	// Allocate the texture
	TextureInitInfo tinit;
	tinit.m_width = size.x();
	tinit.m_height = size.y();
	tinit.m_format = PixelFormat(ComponentFormat::R8, TransformFormat::UNORM);
	tinit.m_mipmapsCount = mipCount;
	tinit.m_sampling.m_minMagFilter = SamplingFilter::LINEAR;
	tinit.m_sampling.m_mipmapFilter = SamplingFilter::LINEAR;

	TexturePtr tex = m_gr->newInstance<Texture>(tinit);

	// Load data
	CommandBufferPtr cmdb = m_gr->newInstance<CommandBuffer>(CommandBufferInitInfo());
	TransientMemoryToken token;
	void* loadData = m_gr->allocateFrameTransientMemory(data.getSize(), BufferUsageBit::TEXTURE_UPLOAD_SOURCE, token);
	memcpy(loadData, &data[0], data.getSize());
	cmdb->uploadTextureSurface(tex, TextureSurfaceInfo(0, 0, 0, 0), token);

	// Gen mips
	cmdb->generateMipmaps2d(tex, 0, 0);
	cmdb->flush();

	// Create the UiImage
	UiImageImpl* impl = getAllocator().newInstance<UiImageImpl>(this);
	impl->m_texture = tex;
	img.reset(impl);
#endif

	return ErrorCode::NONE;
}

Error UiInterfaceImpl::readFile(const CString& filename, DynamicArrayAuto<U8>& data)
{
	GenericResourcePtr rsrc;
	ANKI_CHECK(m_rc->loadResource(filename, rsrc));

	data.create(rsrc->getData().getSize());
	memcpy(&data[0], &rsrc->getData()[0], data.getSize());

	return ErrorCode::NONE;
}

} // end namespace anki
