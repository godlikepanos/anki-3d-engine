// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/ui/Canvas.h>
#include <anki/ui/Font.h>
#include <anki/ui/UiManager.h>
#include <anki/resource/ResourceManager.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/input/Input.h>

namespace anki
{

Canvas::Canvas(UiManager* manager)
	: UiObject(manager)
{
}

Canvas::~Canvas()
{
	nk_free(&m_nkCtx);
	nk_buffer_free(&m_nkCmdsBuff);
}

Error Canvas::init(FontPtr font, U32 fontHeight, U32 width, U32 height)
{
	m_font = font;
	resize(width, height);

	nk_allocator alloc = makeNkAllocator(&getAllocator().getMemoryPool());
	if(!nk_init(&m_nkCtx, &alloc, &m_font->getFont(fontHeight)))
	{
		ANKI_UI_LOGE("nk_init() failed");
		return Error::FUNCTION_FAILED;
	}

	nk_buffer_init(&m_nkCmdsBuff, &alloc, 1_KB);

	// Create program
	ANKI_CHECK(m_manager->getResourceManager().loadResource("shaders/Ui.glslp", m_prog));
	const ShaderProgramResourceVariant* variant;

	for(U i = 0; i < SHADER_COUNT; ++i)
	{
		ShaderProgramResourceMutationInitList<1> mutators(m_prog);
		mutators.add("TEXTURE_TYPE", i);
		m_prog->getOrCreateVariant(mutators.get(), variant);
		m_grProgs[i] = variant->getProgram();
	}

	// Other
	m_stackAlloc = StackAllocator<U8>(getAllocator().getMemoryPool().getAllocationCallback(),
		getAllocator().getMemoryPool().getAllocationCallbackUserData(),
		512_B);

	SamplerInitInfo samplerInit("Canvas");
	samplerInit.m_minMagFilter = SamplingFilter::LINEAR;
	samplerInit.m_mipmapFilter = SamplingFilter::LINEAR;
	samplerInit.m_addressing = SamplingAddressing::REPEAT;
	m_sampler = m_manager->getGrManager().newSampler(samplerInit);

	return Error::NONE;
}

void Canvas::reset()
{
	m_references.destroy(m_stackAlloc);
	m_stackAlloc.getMemoryPool().reset();

	nk_clear(&m_nkCtx);
}

void Canvas::handleInput()
{
	// Start
	const Input& in = m_manager->getInput();
	nk_input_begin(&m_nkCtx);

	Array<U32, 4> viewport = {{0, 0, m_width, m_height}};

	// Handle mouse
	Vec2 mousePosf = in.getMousePosition() / 2.0f + 0.5f;
	mousePosf.y() = 1.0f - mousePosf.y();
	UVec2 mousePos(mousePosf.x() * viewport[2], mousePosf.y() * viewport[3]);

	nk_input_motion(&m_nkCtx, mousePos.x(), mousePos.y());
	nk_input_button(&m_nkCtx, NK_BUTTON_LEFT, mousePos.x(), mousePos.y(), in.getMouseButton(MouseButton::LEFT) > 1);
	nk_input_button(&m_nkCtx, NK_BUTTON_RIGHT, mousePos.x(), mousePos.y(), in.getMouseButton(MouseButton::RIGHT) > 1);

	// Done
	nk_input_end(&m_nkCtx);
}

void Canvas::beginBuilding()
{
#if ANKI_EXTRA_CHECKS
	ANKI_ASSERT(!m_building);
	m_building = true;
#endif
}

void Canvas::endBuilding()
{
#if ANKI_EXTRA_CHECKS
	ANKI_ASSERT(m_building);
	m_building = false;
#endif
}

void Canvas::pushFont(const FontPtr& font, U32 fontHeight)
{
	ANKI_ASSERT(m_building);
	m_references.pushBack(m_stackAlloc, IntrusivePtr<UiObject>(const_cast<Font*>(font.get())));
	nk_style_push_font(&m_nkCtx, &font->getFont(fontHeight));
}

void Canvas::appendToCommandBuffer(CommandBufferPtr cmdb)
{
	Array<U32, 4> viewport = {{0, 0, m_width, m_height}};

	//
	// Allocate vertex data
	//
	struct Vert
	{
		Vec2 m_pos;
		Vec2 m_uv;
		Array<U8, 4> m_col;
	};

	static const struct nk_draw_vertex_layout_element VERT_LAYOUT[] = {
		{NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(Vert, m_pos)},
		{NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(Vert, m_uv)},
		{NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(Vert, m_col)},
		{NK_VERTEX_LAYOUT_END}};

	struct GpuAllocCtx
	{
		StagingGpuMemoryManager* m_alloc;
		StagingGpuMemoryToken m_token;
	};

	auto allocVertDataCallback = [](nk_handle handle, void* old, nk_size size) -> void* {
		GpuAllocCtx* ctx = static_cast<GpuAllocCtx*>(handle.ptr);
		void* out = ctx->m_alloc->allocateFrame(size, StagingGpuMemoryType::VERTEX, ctx->m_token);
		ANKI_ASSERT(out);
		return out;
	};

	auto freeVertDataCallback = [](nk_handle, void*) -> void {
		// Do nothing
	};

	GpuAllocCtx idxCtx;
	idxCtx.m_alloc = &m_manager->getStagingGpuMemoryManager();
	nk_allocator idxAlloc;
	idxAlloc.userdata.ptr = &idxCtx;
	idxAlloc.alloc = allocVertDataCallback;
	idxAlloc.free = freeVertDataCallback;

	nk_buffer idxBuff = {};
	nk_buffer_init(&idxBuff, &idxAlloc, 1_KB);

	GpuAllocCtx vertCtx;
	vertCtx.m_alloc = &m_manager->getStagingGpuMemoryManager();
	nk_allocator vertAlloc;
	vertAlloc.userdata.ptr = &vertCtx;
	vertAlloc.alloc = allocVertDataCallback;
	vertAlloc.free = freeVertDataCallback;

	nk_buffer vertBuff = {};
	nk_buffer_init(&vertBuff, &vertAlloc, 1_KB);

	nk_convert_config config = {};
	config.vertex_layout = VERT_LAYOUT;
	config.vertex_size = sizeof(Vert);
	config.vertex_alignment = NK_ALIGNOF(Vert);
	config.null.texture.ptr = nullptr;
	config.null.uv = {0, 0};
	config.circle_segment_count = 22;
	config.curve_segment_count = 22;
	config.arc_segment_count = 22;
	config.global_alpha = 1.0f;
	config.shape_AA = NK_ANTI_ALIASING_OFF;
	config.line_AA = NK_ANTI_ALIASING_OFF;

	nk_convert(&m_nkCtx, &m_nkCmdsBuff, &vertBuff, &idxBuff, &config);

	//
	// Draw
	//

	// Uniforms
	StagingGpuMemoryToken uboToken;
	Vec4* trf = static_cast<Vec4*>(
		m_manager->getStagingGpuMemoryManager().allocateFrame(sizeof(Vec4), StagingGpuMemoryType::UNIFORM, uboToken));
	trf->x() = 2.0f / (viewport[2] - viewport[0]);
	trf->y() = -2.0f / (viewport[3] - viewport[1]);
	trf->z() = -1.0f;
	trf->w() = 1.0f;
	cmdb->bindUniformBuffer(0, 0, uboToken.m_buffer, uboToken.m_offset, uboToken.m_range);

	// Vert & idx buffers
	cmdb->bindVertexBuffer(0, vertCtx.m_token.m_buffer, vertCtx.m_token.m_offset, sizeof(Vert));
	cmdb->setVertexAttribute(0, 0, Format::R32G32_SFLOAT, 0);
	cmdb->setVertexAttribute(1, 0, Format::R8G8B8A8_UNORM, sizeof(Vec2) * 2);
	cmdb->setVertexAttribute(2, 0, Format::R32G32_SFLOAT, sizeof(Vec2));

	cmdb->bindIndexBuffer(idxCtx.m_token.m_buffer, idxCtx.m_token.m_offset, IndexType::U16);

	cmdb->setViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

	// Prog & tex
	ShaderProgramPtr boundProg;

	const nk_draw_command* cmd = nullptr;
	U offset = 0;
	nk_draw_foreach(cmd, &m_nkCtx, &m_nkCmdsBuff)
	{
		if(!cmd->elem_count)
		{
			continue;
		}

		// Set texture and program
		ShaderProgramPtr progToBind;
		if(cmd->texture.ptr == nullptr)
		{
			progToBind = m_grProgs[NO_TEX];
		}
		else if(ptrToNumber(cmd->texture.ptr) & FONT_TEXTURE_MASK)
		{
			progToBind = m_grProgs[RGBA_TEX];

			TextureView* t = numberToPtr<TextureView*>(ptrToNumber(cmd->texture.ptr) & ~FONT_TEXTURE_MASK);
			cmdb->bindTextureAndSampler(0, 0, TextureViewPtr(t), m_sampler, TextureUsageBit::SAMPLED_FRAGMENT);
		}
		else
		{
			ANKI_ASSERT(!"TODO");
		}

		if(boundProg != progToBind)
		{
			cmdb->bindShaderProgram(progToBind);
			boundProg = progToBind;
		}

		// Other state
		cmdb->setBlendFactors(0, BlendFactor::SRC_ALPHA, BlendFactor::ONE_MINUS_SRC_ALPHA);
		cmdb->setDepthWrite(false);
		cmdb->setDepthCompareOperation(CompareOperation::ALWAYS);
		cmdb->setCullMode(FaceSelectionBit::FRONT);

		// TODO set scissor

		// Draw
		cmdb->drawElements(PrimitiveTopology::TRIANGLES, cmd->elem_count, 1, offset);

		// Advance
		offset += cmd->elem_count;
	}

	//
	// Done
	//
	reset();
}

} // end namespace anki
