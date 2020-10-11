// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/CommandBuffer.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/gl/GrManagerImpl.h>
#include <anki/gr/gl/RenderingThread.h>
#include <anki/gr/gl/GlState.h>

#include <anki/gr/gl/FramebufferImpl.h>
#include <anki/gr/gl/OcclusionQueryImpl.h>
#include <anki/gr/gl/TextureImpl.h>
#include <anki/gr/gl/BufferImpl.h>
#include <anki/gr/gl/SamplerImpl.h>
#include <anki/gr/gl/ShaderProgramImpl.h>
#include <anki/gr/gl/TextureViewImpl.h>

#include <anki/core/Trace.h>

namespace anki
{

CommandBuffer* CommandBuffer::newInstance(GrManager* manager, const CommandBufferInitInfo& inf)
{
	CommandBufferImpl* impl = manager->getAllocator().newInstance<CommandBufferImpl>(manager, inf.getName());
	impl->init(inf);
	return impl;
}

void CommandBuffer::flush(FencePtr* fence)
{
	ANKI_GL_SELF(CommandBufferImpl);

	if(!self.isSecondLevel())
	{
		ANKI_ASSERT(!self.m_state.insideRenderPass());
	}
	else
	{
		ANKI_ASSERT(fence == nullptr);
	}

	if(!self.isSecondLevel())
	{
		static_cast<GrManagerImpl&>(getManager())
			.getRenderingThread()
			.flushCommandBuffer(CommandBufferPtr(this), fence);
	}
}

void CommandBuffer::bindVertexBuffer(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride,
									 VertexStepRate stepRate)
{
	class Cmd final : public GlCommand
	{
	public:
		BufferPtr m_buff;
		U32 m_binding;
		PtrSize m_offset;
		PtrSize m_stride;
		Bool m_instanced;

		Cmd(U32 binding, BufferPtr buff, PtrSize offset, PtrSize stride, Bool instanced)
			: m_buff(buff)
			, m_binding(binding)
			, m_offset(offset)
			, m_stride(stride)
			, m_instanced(instanced)
		{
		}

		Error operator()(GlState& state)
		{
			glBindVertexBuffer(m_binding, static_cast<const BufferImpl&>(*m_buff).getGlName(), m_offset, m_stride);
			glVertexBindingDivisor(m_binding, (m_instanced) ? 1 : 0);
			return Error::NONE;
		}
	};

	ANKI_ASSERT(buff);
	ANKI_ASSERT(stride > 0);
	ANKI_GL_SELF(CommandBufferImpl);

	if(self.m_state.bindVertexBuffer(binding, buff, offset, stride, stepRate))
	{
		self.pushBackNewCommand<Cmd>(binding, buff, offset, stride, stepRate == VertexStepRate::INSTANCE);
	}
}

void CommandBuffer::setVertexAttribute(U32 location, U32 buffBinding, Format fmt, PtrSize relativeOffset)
{
	class Cmd final : public GlCommand
	{
	public:
		U32 m_location;
		U32 m_buffBinding;
		U8 m_compSize;
		GLenum m_fmt;
		Bool m_normalized;
		PtrSize m_relativeOffset;

		Cmd(U32 location, U32 buffBinding, U8 compSize, GLenum fmt, Bool normalized, PtrSize relativeOffset)
			: m_location(location)
			, m_buffBinding(buffBinding)
			, m_compSize(compSize)
			, m_fmt(fmt)
			, m_normalized(normalized)
			, m_relativeOffset(relativeOffset)
		{
		}

		Error operator()(GlState& state)
		{
			glVertexAttribFormat(m_location, m_compSize, m_fmt, m_normalized, m_relativeOffset);
			glVertexAttribBinding(m_location, m_buffBinding);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);

	if(self.m_state.setVertexAttribute(location, buffBinding, fmt, relativeOffset))
	{
		U compCount;
		GLenum type;
		Bool normalized;

		convertVertexFormat(fmt, compCount, type, normalized);

		self.pushBackNewCommand<Cmd>(location, buffBinding, compCount, type, normalized, relativeOffset);
	}
}

void CommandBuffer::bindIndexBuffer(BufferPtr buff, PtrSize offset, IndexType type)
{
	class Cmd final : public GlCommand
	{
	public:
		BufferPtr m_buff;

		Cmd(BufferPtr buff)
			: m_buff(buff)
		{
		}

		Error operator()(GlState& state)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, static_cast<const BufferImpl&>(*m_buff).getGlName());
			return Error::NONE;
		}
	};

	ANKI_ASSERT(buff);
	ANKI_GL_SELF(CommandBufferImpl);

	if(self.m_state.bindIndexBuffer(buff, offset, type))
	{
		self.pushBackNewCommand<Cmd>(buff);
	}
}

void CommandBuffer::setPrimitiveRestart(Bool enable)
{
	class Cmd final : public GlCommand
	{
	public:
		Bool m_enable;

		Cmd(Bool enable)
			: m_enable(enable)
		{
		}

		Error operator()(GlState& state)
		{
			if(m_enable)
			{
				glEnable(GL_PRIMITIVE_RESTART);
			}
			else
			{
				glDisable(GL_PRIMITIVE_RESTART);
			}

			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setPrimitiveRestart(enable))
	{
		self.pushBackNewCommand<Cmd>(enable);
	}
}

void CommandBuffer::setViewport(U32 minx, U32 miny, U32 width, U32 height)
{
	class ViewportCommand final : public GlCommand
	{
	public:
		Array<U32, 4> m_value;

		ViewportCommand(U32 a, U32 b, U32 c, U32 d)
		{
			m_value = {{a, b, c, d}};
		}

		Error operator()(GlState& state)
		{
			glViewport(m_value[0], m_value[1], m_value[2], m_value[3]);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setViewport(minx, miny, width, height))
	{
		self.pushBackNewCommand<ViewportCommand>(minx, miny, width, height);
	}
}

void CommandBuffer::setScissor(U32 minx, U32 miny, U32 width, U32 height)
{
	ANKI_ASSERT(minx < MAX_U32 && miny < MAX_U32);
	ANKI_ASSERT(width > 0 && height > 0);

	class ScissorCommand final : public GlCommand
	{
	public:
		Array<GLsizei, 4> m_value;

		ScissorCommand(GLsizei a, GLsizei b, GLsizei c, GLsizei d)
		{
			m_value = {{a, b, c, d}};
		}

		Error operator()(GlState& state)
		{
			if(state.m_scissor[0] != m_value[0] || state.m_scissor[1] != m_value[1] || state.m_scissor[2] != m_value[2]
			   || state.m_scissor[3] != m_value[3])
			{
				state.m_scissor = m_value;
				glScissor(m_value[0], m_value[1], m_value[2], m_value[3]);
			}
			return Error::NONE;
		}
	};

	// Limit the width and height to GLsizei
	const GLsizei iwidth = (width == MAX_U32) ? MAX_I32 : width;
	const GLsizei iheight = (height == MAX_U32) ? MAX_I32 : height;
	const GLsizei iminx = minx;
	const GLsizei iminy = miny;

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setScissor(iminx, iminy, iwidth, iheight))
	{
		self.pushBackNewCommand<ScissorCommand>(iminx, iminy, iwidth, iheight);
	}
}

void CommandBuffer::setFillMode(FillMode mode)
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_fillMode;

		Cmd(GLenum fillMode)
			: m_fillMode(fillMode)
		{
		}

		Error operator()(GlState& state)
		{
			glPolygonMode(GL_FRONT_AND_BACK, m_fillMode);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setFillMode(mode))
	{
		self.pushBackNewCommand<Cmd>(convertFillMode(mode));
	}
}

void CommandBuffer::setCullMode(FaceSelectionBit mode)
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_mode;

		Cmd(GLenum mode)
			: m_mode(mode)
		{
		}

		Error operator()(GlState& state)
		{
			glCullFace(m_mode);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setCullMode(mode))
	{
		self.pushBackNewCommand<Cmd>(convertFaceMode(mode));
	}
}

void CommandBuffer::setPolygonOffset(F32 factor, F32 units)
{
	class Cmd final : public GlCommand
	{
	public:
		F32 m_factor;
		F32 m_units;

		Cmd(F32 factor, F32 units)
			: m_factor(factor)
			, m_units(units)
		{
		}

		Error operator()(GlState& state)
		{
			if(m_factor == 0.0 && m_units == 0.0)
			{
				glDisable(GL_POLYGON_OFFSET_FILL);
			}
			else
			{
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(m_factor, m_units);
			}

			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setPolygonOffset(factor, units))
	{
		self.pushBackNewCommand<Cmd>(factor, units);
	}
}

void CommandBuffer::setStencilOperations(FaceSelectionBit face, StencilOperation stencilFail,
										 StencilOperation stencilPassDepthFail, StencilOperation stencilPassDepthPass)
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_face;
		GLenum m_stencilFail;
		GLenum m_stencilPassDepthFail;
		GLenum m_stencilPassDepthPass;

		Cmd(GLenum face, GLenum stencilFail, GLenum stencilPassDepthFail, GLenum stencilPassDepthPass)
			: m_face(face)
			, m_stencilFail(stencilFail)
			, m_stencilPassDepthFail(stencilPassDepthFail)
			, m_stencilPassDepthPass(stencilPassDepthPass)
		{
		}

		Error operator()(GlState& state)
		{
			glStencilOpSeparate(m_face, m_stencilFail, m_stencilPassDepthFail, m_stencilPassDepthPass);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setStencilOperations(face, stencilFail, stencilPassDepthFail, stencilPassDepthPass))
	{
		self.pushBackNewCommand<Cmd>(convertFaceMode(face), convertStencilOperation(stencilFail),
									 convertStencilOperation(stencilPassDepthFail),
									 convertStencilOperation(stencilPassDepthPass));
	}
}

void CommandBuffer::setStencilCompareOperation(FaceSelectionBit face, CompareOperation comp)
{
	ANKI_GL_SELF(CommandBufferImpl);
	self.m_state.setStencilCompareOperation(face, comp);
}

void CommandBuffer::setStencilCompareMask(FaceSelectionBit face, U32 mask)
{
	ANKI_GL_SELF(CommandBufferImpl);
	self.m_state.setStencilCompareMask(face, mask);
}

void CommandBuffer::setStencilWriteMask(FaceSelectionBit face, U32 mask)
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_face;
		U32 m_mask;

		Cmd(GLenum face, U32 mask)
			: m_face(face)
			, m_mask(mask)
		{
		}

		Error operator()(GlState& state)
		{
			glStencilMaskSeparate(m_face, m_mask);

			if(m_face == GL_FRONT)
			{
				state.m_stencilWriteMask[0] = m_mask;
			}
			else if(m_face == GL_BACK)
			{
				state.m_stencilWriteMask[1] = m_mask;
			}
			else
			{
				ANKI_ASSERT(m_face == GL_FRONT_AND_BACK);
				state.m_stencilWriteMask[0] = state.m_stencilWriteMask[1] = m_mask;
			}

			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setStencilWriteMask(face, mask))
	{
		self.pushBackNewCommand<Cmd>(convertFaceMode(face), mask);
	}
}

void CommandBuffer::setStencilReference(FaceSelectionBit face, U32 ref)
{
	ANKI_GL_SELF(CommandBufferImpl);
	self.m_state.setStencilReference(face, ref);
}

void CommandBuffer::setDepthWrite(Bool enable)
{
	class Cmd final : public GlCommand
	{
	public:
		Bool m_enable;

		Cmd(Bool enable)
			: m_enable(enable)
		{
		}

		Error operator()(GlState& state)
		{
			glDepthMask(m_enable);
			state.m_depthWriteMask = m_enable;
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setDepthWrite(enable))
	{
		self.pushBackNewCommand<Cmd>(enable);
	}
}

void CommandBuffer::setDepthCompareOperation(CompareOperation op)
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_op;

		Cmd(GLenum op)
			: m_op(op)
		{
		}

		Error operator()(GlState& state)
		{
			glDepthFunc(m_op);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setDepthCompareOperation(op))
	{
		self.pushBackNewCommand<Cmd>(convertCompareOperation(op));
	}
}

void CommandBuffer::setAlphaToCoverage(Bool enable)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setColorChannelWriteMask(U32 attachment, ColorBit mask)
{
	class Cmd final : public GlCommand
	{
	public:
		U8 m_attachment;
		ColorBit m_mask;

		Cmd(U8 attachment, ColorBit mask)
			: m_attachment(attachment)
			, m_mask(mask)
		{
		}

		Error operator()(GlState& state)
		{
			const Bool r = !!(m_mask & ColorBit::RED);
			const Bool g = !!(m_mask & ColorBit::GREEN);
			const Bool b = !!(m_mask & ColorBit::BLUE);
			const Bool a = !!(m_mask & ColorBit::ALPHA);

			glColorMaski(m_attachment, r, g, b, a);

			state.m_colorWriteMasks[m_attachment] = {{r, g, b, a}};

			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setColorChannelWriteMask(attachment, mask))
	{
		self.pushBackNewCommand<Cmd>(attachment, mask);
	}
}

void CommandBuffer::setBlendFactors(U32 attachment, BlendFactor srcRgb, BlendFactor dstRgb, BlendFactor srcA,
									BlendFactor dstA)
{
	class Cmd final : public GlCommand
	{
	public:
		U8 m_attachment;
		GLenum m_srcRgb;
		GLenum m_dstRgb;
		GLenum m_srcA;
		GLenum m_dstA;

		Cmd(U8 att, GLenum srcRgb, GLenum dstRgb, GLenum srcA, GLenum dstA)
			: m_attachment(att)
			, m_srcRgb(srcRgb)
			, m_dstRgb(dstRgb)
			, m_srcA(srcA)
			, m_dstA(dstA)
		{
		}

		Error operator()(GlState&)
		{
			glBlendFuncSeparatei(m_attachment, m_srcRgb, m_dstRgb, m_srcA, m_dstA);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setBlendFactors(attachment, srcRgb, dstRgb, srcA, dstA))
	{
		self.pushBackNewCommand<Cmd>(attachment, convertBlendFactor(srcRgb), convertBlendFactor(dstRgb),
									 convertBlendFactor(srcA), convertBlendFactor(dstA));
	}
}

void CommandBuffer::setBlendOperation(U32 attachment, BlendOperation funcRgb, BlendOperation funcA)
{
	class Cmd final : public GlCommand
	{
	public:
		U8 m_attachment;
		GLenum m_funcRgb;
		GLenum m_funcA;

		Cmd(U8 att, GLenum funcRgb, GLenum funcA)
			: m_attachment(att)
			, m_funcRgb(funcRgb)
			, m_funcA(funcA)
		{
		}

		Error operator()(GlState&)
		{
			glBlendEquationSeparatei(m_attachment, m_funcRgb, m_funcA);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.setBlendOperation(attachment, funcRgb, funcA))
	{
		self.pushBackNewCommand<Cmd>(attachment, convertBlendOperation(funcRgb), convertBlendOperation(funcA));
	}
}

void CommandBuffer::bindTextureAndSampler(U32 set, U32 binding, TextureViewPtr texView, SamplerPtr sampler,
										  TextureUsageBit usage)
{
	class Cmd final : public GlCommand
	{
	public:
		U32 m_unit;
		TextureViewPtr m_texView;
		SamplerPtr m_sampler;

		Cmd(U32 unit, TextureViewPtr texView, SamplerPtr sampler)
			: m_unit(unit)
			, m_texView(texView)
			, m_sampler(sampler)
		{
		}

		Error operator()(GlState&)
		{
			glBindTextureUnit(m_unit, static_cast<const TextureViewImpl&>(*m_texView).m_view.m_glName);
			glBindSampler(m_unit, static_cast<const SamplerImpl&>(*m_sampler).getGlName());
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(static_cast<const TextureViewImpl&>(*texView).m_tex->isSubresourceGoodForSampling(
		static_cast<const TextureViewImpl&>(*texView).getSubresource()));

	if(self.m_state.bindTextureViewAndSampler(set, binding, texView, sampler))
	{
		U unit = binding + MAX_TEXTURE_BINDINGS * set;
		self.pushBackNewCommand<Cmd>(unit, texView, sampler);
	}
}

void CommandBuffer::bindUniformBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
{
	class Cmd final : public GlCommand
	{
	public:
		BufferPtr m_buff;
		PtrSize m_binding;
		PtrSize m_offset;
		PtrSize m_range;

		Cmd(U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
			: m_buff(buff)
			, m_binding(binding)
			, m_offset(offset)
			, m_range(range)
		{
		}

		Error operator()(GlState&)
		{
			static_cast<const BufferImpl&>(*m_buff).bind(GL_UNIFORM_BUFFER, m_binding, m_offset, m_range);
			return Error::NONE;
		}
	};

	ANKI_ASSERT(buff);
	ANKI_ASSERT(range > 0);
	ANKI_GL_SELF(CommandBufferImpl);

	if(self.m_state.bindUniformBuffer(set, binding, buff, offset, range))
	{
		binding = binding + MAX_UNIFORM_BUFFER_BINDINGS * set;
		self.pushBackNewCommand<Cmd>(binding, buff, offset, range);
	}
}

void CommandBuffer::bindStorageBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
{
	class Cmd final : public GlCommand
	{
	public:
		BufferPtr m_buff;
		PtrSize m_binding;
		PtrSize m_offset;
		PtrSize m_range;

		Cmd(U32 binding, BufferPtr buff, PtrSize offset, PtrSize range)
			: m_buff(buff)
			, m_binding(binding)
			, m_offset(offset)
			, m_range(range)
		{
		}

		Error operator()(GlState&)
		{
			static_cast<const BufferImpl&>(*m_buff).bind(GL_SHADER_STORAGE_BUFFER, m_binding, m_offset, m_range);
			return Error::NONE;
		}
	};

	ANKI_ASSERT(buff);
	ANKI_ASSERT(range > 0);
	ANKI_GL_SELF(CommandBufferImpl);

	if(self.m_state.bindStorageBuffer(set, binding, buff, offset, range))
	{
		binding = binding + MAX_STORAGE_BUFFER_BINDINGS * set;
		self.pushBackNewCommand<Cmd>(binding, buff, offset, range);
	}
}

void CommandBuffer::bindImage(U32 set, U32 binding, TextureViewPtr img)
{
	class Cmd final : public GlCommand
	{
	public:
		TextureViewPtr m_img;
		U16 m_unit;

		Cmd(U32 unit, TextureViewPtr img)
			: m_img(img)
			, m_unit(unit)
		{
		}

		Error operator()(GlState&)
		{
			const TextureViewImpl& view = static_cast<const TextureViewImpl&>(*m_img);

			glBindImageTexture(m_unit, view.m_view.m_glName, 0, GL_TRUE, 0, GL_READ_WRITE,
							   static_cast<const TextureImpl&>(*view.m_tex).m_internalFormat);
			return Error::NONE;
		}
	};

	ANKI_ASSERT(img);
	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(static_cast<const TextureViewImpl&>(*img).m_tex->isSubresourceGoodForImageLoadStore(
		static_cast<const TextureViewImpl&>(*img).getSubresource()));

	if(self.m_state.bindImage(set, binding, img))
	{
		binding = binding + set * MAX_IMAGE_BINDINGS;
		self.pushBackNewCommand<Cmd>(binding, img);
	}
}

void CommandBuffer::bindTextureBuffer(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range, Format fmt)
{
	class Cmd final : public GlCommand
	{
	public:
		U32 m_set;
		U32 m_binding;
		BufferPtr m_buff;
		PtrSize m_offset;
		PtrSize m_range;
		GLenum m_fmt;

		Cmd(U32 set, U32 binding, BufferPtr buff, PtrSize offset, PtrSize range, GLenum fmt)
			: m_set(set)
			, m_binding(binding)
			, m_buff(buff)
			, m_offset(offset)
			, m_range(range)
			, m_fmt(fmt)
		{
		}

		Error operator()(GlState& state)
		{
			ANKI_ASSERT(m_offset + m_range <= m_buff->getSize());

			const GLuint tex = state.m_texBuffTextures[m_set][m_binding];
			glTextureBufferRange(tex, m_fmt, static_cast<const BufferImpl&>(*m_buff).getGlName(), m_offset, m_range);

			return Error::NONE;
		}
	};

	Bool compressed;
	GLenum format;
	GLenum internalFormat;
	GLenum type;
	DepthStencilAspectBit dsAspect;
	convertTextureInformation(fmt, compressed, format, internalFormat, type, dsAspect);
	(void)compressed;
	(void)format;
	(void)type;
	(void)dsAspect;

	ANKI_GL_SELF(CommandBufferImpl);
	self.pushBackNewCommand<Cmd>(set, binding, buff, offset, range, internalFormat);
}

void CommandBuffer::bindShaderProgram(ShaderProgramPtr prog)
{
	class Cmd final : public GlCommand
	{
	public:
		ShaderProgramPtr m_prog;

		Cmd(const ShaderProgramPtr& prog)
			: m_prog(prog)
		{
		}

		Error operator()(GlState& state)
		{
			state.m_crntProg = m_prog;
			glUseProgram(static_cast<const ShaderProgramImpl&>(*m_prog).getGlName());
			return Error::NONE;
		}
	};

	ANKI_ASSERT(prog);
	ANKI_GL_SELF(CommandBufferImpl);

	if(self.m_state.bindShaderProgram(prog))
	{
		self.pushBackNewCommand<Cmd>(prog);
	}
	else
	{
		ANKI_TRACE_INC_COUNTER(GL_PROGS_SKIPPED, 1);
	}
}

void CommandBuffer::beginRenderPass(FramebufferPtr fb,
									const Array<TextureUsageBit, MAX_COLOR_ATTACHMENTS>& colorAttachmentUsages,
									TextureUsageBit depthStencilAttachmentUsage, U32 minx, U32 miny, U32 width,
									U32 height)
{
	class BindFramebufferCommand final : public GlCommand
	{
	public:
		FramebufferPtr m_fb;
		Array<U32, 4> m_renderArea;

		BindFramebufferCommand(FramebufferPtr fb, U32 minx, U32 miny, U32 width, U32 height)
			: m_fb(fb)
			, m_renderArea{{minx, miny, width, height}}
		{
		}

		Error operator()(GlState& state)
		{
			static_cast<const FramebufferImpl&>(*m_fb).bind(state, m_renderArea[0], m_renderArea[1], m_renderArea[2],
															m_renderArea[3]);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	if(self.m_state.beginRenderPass(fb))
	{
		self.pushBackNewCommand<BindFramebufferCommand>(fb, minx, miny, width, height);
	}
}

void CommandBuffer::endRenderPass()
{
	class Command final : public GlCommand
	{
	public:
		const FramebufferImpl* m_fb;

		Command(const FramebufferImpl* fb)
			: m_fb(fb)
		{
			ANKI_ASSERT(fb);
		}

		Error operator()(GlState&)
		{
			m_fb->endRenderPass();
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	self.pushBackNewCommand<Command>(self.m_state.m_fb);
	self.m_state.endRenderPass();
}

void CommandBuffer::drawElements(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 firstIndex,
								 U32 baseVertex, U32 baseInstance)
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_topology;
		GLenum m_indexType;
		DrawElementsIndirectInfo m_info;

		Cmd(GLenum topology, GLenum indexType, const DrawElementsIndirectInfo& info)
			: m_topology(topology)
			, m_indexType(indexType)
			, m_info(info)
		{
		}

		Error operator()(GlState&)
		{
			glDrawElementsInstancedBaseVertexBaseInstance(
				m_topology, m_info.m_count, m_indexType, numberToPtr<void*>(m_info.m_firstIndex),
				m_info.m_instanceCount, m_info.m_baseVertex, m_info.m_baseInstance);

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
			ANKI_TRACE_INC_COUNTER(GR_VERTICES, m_info.m_instanceCount * m_info.m_count);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);

	self.m_state.checkIndexedDracall();
	self.flushDrawcall(*this);

	U idxBytes;
	if(self.m_state.m_idx.m_indexType == GL_UNSIGNED_SHORT)
	{
		idxBytes = sizeof(U16);
	}
	else
	{
		ANKI_ASSERT(self.m_state.m_idx.m_indexType == GL_UNSIGNED_INT);
		idxBytes = sizeof(U32);
	}

	firstIndex = firstIndex * idxBytes + self.m_state.m_idx.m_offset;

	DrawElementsIndirectInfo info(count, instanceCount, firstIndex, baseVertex, baseInstance);
	self.pushBackNewCommand<Cmd>(convertPrimitiveTopology(topology), self.m_state.m_idx.m_indexType, info);
}

void CommandBuffer::drawArrays(PrimitiveTopology topology, U32 count, U32 instanceCount, U32 first, U32 baseInstance)
{
	class DrawArraysCommand final : public GlCommand
	{
	public:
		GLenum m_topology;
		DrawArraysIndirectInfo m_info;

		DrawArraysCommand(GLenum topology, const DrawArraysIndirectInfo& info)
			: m_topology(topology)
			, m_info(info)
		{
		}

		Error operator()(GlState& state)
		{
			glDrawArraysInstancedBaseInstance(m_topology, m_info.m_first, m_info.m_count, m_info.m_instanceCount,
											  m_info.m_baseInstance);

			ANKI_TRACE_INC_COUNTER(GR_DRAWCALLS, 1);
			ANKI_TRACE_INC_COUNTER(GR_VERTICES, m_info.m_instanceCount * m_info.m_count);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);

	self.m_state.checkNonIndexedDrawcall();
	self.flushDrawcall(*this);

	DrawArraysIndirectInfo info(count, instanceCount, first, baseInstance);
	self.pushBackNewCommand<DrawArraysCommand>(convertPrimitiveTopology(topology), info);
}

void CommandBuffer::drawElementsIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset,
										 BufferPtr indirectBuff)
{
	class DrawElementsIndirectCommand final : public GlCommand
	{
	public:
		GLenum m_topology;
		GLenum m_indexType;
		U32 m_drawCount;
		PtrSize m_offset;
		BufferPtr m_buff;

		DrawElementsIndirectCommand(GLenum topology, GLenum indexType, U32 drawCount, PtrSize offset, BufferPtr buff)
			: m_topology(topology)
			, m_indexType(indexType)
			, m_drawCount(drawCount)
			, m_offset(offset)
			, m_buff(buff)
		{
			ANKI_ASSERT(drawCount > 0);
			ANKI_ASSERT((m_offset % 4) == 0);
		}

		Error operator()(GlState&)
		{
			const BufferImpl& buff = static_cast<const BufferImpl&>(*m_buff);

			ANKI_ASSERT(m_offset + sizeof(DrawElementsIndirectInfo) * m_drawCount <= buff.getSize());

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buff.getGlName());

			glMultiDrawElementsIndirect(m_topology, m_indexType, numberToPtr<void*>(m_offset), m_drawCount,
										sizeof(DrawElementsIndirectInfo));

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);

	self.m_state.checkIndexedDracall();
	self.flushDrawcall(*this);
	self.pushBackNewCommand<DrawElementsIndirectCommand>(
		convertPrimitiveTopology(topology), self.m_state.m_idx.m_indexType, drawCount, offset, indirectBuff);
}

void CommandBuffer::drawArraysIndirect(PrimitiveTopology topology, U32 drawCount, PtrSize offset,
									   BufferPtr indirectBuff)
{
	class DrawArraysIndirectCommand final : public GlCommand
	{
	public:
		GLenum m_topology;
		U32 m_drawCount;
		PtrSize m_offset;
		BufferPtr m_buff;

		DrawArraysIndirectCommand(GLenum topology, U32 drawCount, PtrSize offset, BufferPtr buff)
			: m_topology(topology)
			, m_drawCount(drawCount)
			, m_offset(offset)
			, m_buff(buff)
		{
			ANKI_ASSERT(drawCount > 0);
			ANKI_ASSERT((m_offset % 4) == 0);
		}

		Error operator()(GlState& state)
		{
			const BufferImpl& buff = static_cast<const BufferImpl&>(*m_buff);

			ANKI_ASSERT(m_offset + sizeof(DrawArraysIndirectInfo) * m_drawCount <= buff.getSize());

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buff.getGlName());

			glMultiDrawArraysIndirect(m_topology, numberToPtr<void*>(m_offset), m_drawCount,
									  sizeof(DrawArraysIndirectInfo));

			glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	self.m_state.checkNonIndexedDrawcall();
	self.flushDrawcall(*this);
	self.pushBackNewCommand<DrawArraysIndirectCommand>(convertPrimitiveTopology(topology), drawCount, offset,
													   indirectBuff);
}

void CommandBuffer::dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ)
{
	class DispatchCommand final : public GlCommand
	{
	public:
		Array<U32, 3> m_size;

		DispatchCommand(U32 x, U32 y, U32 z)
			: m_size({{x, y, z}})
		{
		}

		Error operator()(GlState&)
		{
			glDispatchCompute(m_size[0], m_size[1], m_size[2]);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);

	ANKI_ASSERT(!!(self.m_flags & CommandBufferFlag::COMPUTE_WORK));
	self.m_state.checkDispatch();
	self.pushBackNewCommand<DispatchCommand>(groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::resetOcclusionQuery(OcclusionQueryPtr query)
{
	// Nothing for GL
}

void CommandBuffer::beginOcclusionQuery(OcclusionQueryPtr query)
{
	class OqBeginCommand final : public GlCommand
	{
	public:
		OcclusionQueryPtr m_handle;

		OqBeginCommand(const OcclusionQueryPtr& handle)
			: m_handle(handle)
		{
		}

		Error operator()(GlState&)
		{
			static_cast<OcclusionQueryImpl&>(*m_handle).begin();
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	self.pushBackNewCommand<OqBeginCommand>(query);
}

void CommandBuffer::endOcclusionQuery(OcclusionQueryPtr query)
{
	class OqEndCommand final : public GlCommand
	{
	public:
		OcclusionQueryPtr m_handle;

		OqEndCommand(const OcclusionQueryPtr& handle)
			: m_handle(handle)
		{
		}

		Error operator()(GlState&)
		{
			static_cast<OcclusionQueryImpl&>(*m_handle).end();
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	self.pushBackNewCommand<OqEndCommand>(query);
}

void CommandBuffer::copyBufferToTextureView(BufferPtr buff, PtrSize offset, PtrSize range, TextureViewPtr texView)
{
	class TexSurfUploadCommand final : public GlCommand
	{
	public:
		BufferPtr m_buff;
		PtrSize m_offset;
		PtrSize m_range;
		TextureViewPtr m_texView;

		TexSurfUploadCommand(BufferPtr buff, PtrSize offset, PtrSize range, TextureViewPtr texView)
			: m_buff(buff)
			, m_offset(offset)
			, m_range(range)
			, m_texView(texView)
		{
		}

		Error operator()(GlState&)
		{
			const TextureViewImpl& viewImpl = static_cast<TextureViewImpl&>(*m_texView);
			const TextureImpl& texImpl = static_cast<TextureImpl&>(*viewImpl.m_tex);

			texImpl.copyFromBuffer(viewImpl.getSubresource(), static_cast<const BufferImpl&>(*m_buff).getGlName(),
								   m_offset, m_range);
			return Error::NONE;
		}
	};

	ANKI_ASSERT(texView);
	ANKI_ASSERT(buff);
	ANKI_ASSERT(range > 0);
	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(!self.m_state.insideRenderPass());

	self.pushBackNewCommand<TexSurfUploadCommand>(buff, offset, range, texView);
}

void CommandBuffer::copyBufferToBuffer(BufferPtr src, PtrSize srcOffset, BufferPtr dst, PtrSize dstOffset,
									   PtrSize range)
{
	class Cmd final : public GlCommand
	{
	public:
		BufferPtr m_src;
		PtrSize m_srcOffset;
		BufferPtr m_dst;
		PtrSize m_dstOffset;
		PtrSize m_range;

		Cmd(BufferPtr src, PtrSize srcOffset, BufferPtr dst, PtrSize dstOffset, PtrSize range)
			: m_src(src)
			, m_srcOffset(srcOffset)
			, m_dst(dst)
			, m_dstOffset(dstOffset)
			, m_range(range)
		{
		}

		Error operator()(GlState& state)
		{
			static_cast<BufferImpl&>(*m_dst).write(static_cast<const BufferImpl&>(*m_src).getGlName(), m_srcOffset,
												   m_dstOffset, m_range);
			return Error::NONE;
		}
	};

	ANKI_ASSERT(src);
	ANKI_ASSERT(dst);
	ANKI_ASSERT(range > 0);
	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(!self.m_state.insideRenderPass());

	self.pushBackNewCommand<Cmd>(src, srcOffset, dst, dstOffset, range);
}

void CommandBuffer::generateMipmaps2d(TextureViewPtr texView)
{
	class GenMipsCommand final : public GlCommand
	{
	public:
		TextureViewPtr m_texView;

		GenMipsCommand(const TextureViewPtr& view)
			: m_texView(view)
		{
		}

		Error operator()(GlState&)
		{
			const TextureViewImpl& viewImpl = static_cast<TextureViewImpl&>(*m_texView);
			const TextureImpl& texImpl = static_cast<TextureImpl&>(*viewImpl.m_tex);

			texImpl.generateMipmaps2d(viewImpl);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(!self.m_state.insideRenderPass());
	self.pushBackNewCommand<GenMipsCommand>(texView);
}

void CommandBuffer::generateMipmaps3d(TextureViewPtr tex)
{
	ANKI_ASSERT(!!"TODO");
}

void CommandBuffer::pushSecondLevelCommandBuffer(CommandBufferPtr cmdb)
{
	class ExecCmdbCommand final : public GlCommand
	{
	public:
		CommandBufferPtr m_cmdb;

		ExecCmdbCommand(const CommandBufferPtr& cmdb)
			: m_cmdb(cmdb)
		{
		}

		Error operator()(GlState&)
		{
			ANKI_TRACE_SCOPED_EVENT(GL_2ND_LEVEL_CMD_BUFFER);
			return static_cast<CommandBufferImpl&>(*m_cmdb).executeAllCommands();
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	self.m_state.m_lastSecondLevelCmdb = static_cast<CommandBufferImpl*>(cmdb.get());
	self.pushBackNewCommand<ExecCmdbCommand>(cmdb);
}

Bool CommandBuffer::isEmpty() const
{
	ANKI_GL_SELF_CONST(CommandBufferImpl);
	return self.isEmpty();
}

void CommandBuffer::blitTextureViews(TextureViewPtr srcView, TextureViewPtr destView)
{
	ANKI_ASSERT(!"TODO");
}

void CommandBuffer::setBufferBarrier(BufferPtr buff, BufferUsageBit prevUsage, BufferUsageBit nextUsage, PtrSize offset,
									 PtrSize size)
{
	class SetBufferMemBarrierCommand final : public GlCommand
	{
	public:
		GLenum m_barrier;

		SetBufferMemBarrierCommand(GLenum barrier)
			: m_barrier(barrier)
		{
		}

		Error operator()(GlState&)
		{
			glMemoryBarrier(m_barrier);
			return Error::NONE;
		}
	};

	GLenum d = GL_NONE;
	BufferUsageBit all = prevUsage | nextUsage;

	if(!!(all & BufferUsageBit::UNIFORM_ALL))
	{
		d |= GL_UNIFORM_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::STORAGE_ALL))
	{
		d |= GL_SHADER_STORAGE_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::INDEX))
	{
		d |= GL_ELEMENT_ARRAY_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::VERTEX))
	{
		d |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::INDIRECT_ALL))
	{
		d |= GL_COMMAND_BARRIER_BIT;
	}

	if(!!(all
		  & (BufferUsageBit::FILL | BufferUsageBit::BUFFER_UPLOAD_SOURCE | BufferUsageBit::BUFFER_UPLOAD_DESTINATION)))
	{
		d |= GL_BUFFER_UPDATE_BARRIER_BIT;
	}

	if(!!(all & BufferUsageBit::QUERY_RESULT))
	{
		d |= GL_QUERY_BUFFER_BARRIER_BIT;
	}

	ANKI_ASSERT(d);
	ANKI_GL_SELF(CommandBufferImpl);
	self.pushBackNewCommand<SetBufferMemBarrierCommand>(d);
}

void CommandBuffer::setTextureSurfaceBarrier(TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage,
											 const TextureSurfaceInfo& surf)
{
	TextureSubresourceInfo subresource;
	setTextureBarrier(tex, prevUsage, nextUsage, subresource);
}

void CommandBuffer::setTextureVolumeBarrier(TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage,
											const TextureVolumeInfo& vol)
{
	TextureSubresourceInfo subresource;
	setTextureBarrier(tex, prevUsage, nextUsage, subresource);
}

void CommandBuffer::setTextureBarrier(TexturePtr tex, TextureUsageBit prevUsage, TextureUsageBit nextUsage,
									  const TextureSubresourceInfo& subresource)
{
	class Cmd final : public GlCommand
	{
	public:
		GLenum m_barrier;

		Cmd(GLenum barrier)
			: m_barrier(barrier)
		{
		}

		Error operator()(GlState&)
		{
			glMemoryBarrier(m_barrier);
			return Error::NONE;
		}
	};

	const TextureUsageBit usage = nextUsage;
	GLenum e = 0;

	if(!!(usage & TextureUsageBit::SAMPLED_ALL))
	{
		e |= GL_TEXTURE_FETCH_BARRIER_BIT;
	}

	if(!!(usage & TextureUsageBit::IMAGE_ALL))
	{
		e |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
	}

	if(!!(usage & TextureUsageBit::TRANSFER_DESTINATION))
	{
		e |= GL_TEXTURE_UPDATE_BARRIER_BIT;
	}

	if(!!(usage & TextureUsageBit::FRAMEBUFFER_ATTACHMENT_READ_WRITE))
	{

		e |= GL_FRAMEBUFFER_BARRIER_BIT;
	}

	if(!!(usage & TextureUsageBit::CLEAR))
	{
		// No idea
	}

	if(!!(usage & TextureUsageBit::GENERATE_MIPMAPS))
	{
		// No idea
	}

	if(e != 0)
	{
		ANKI_GL_SELF(CommandBufferImpl);
		self.pushBackNewCommand<Cmd>(e);
	}
}

void CommandBuffer::clearTextureView(TextureViewPtr texView, const ClearValue& clearValue)
{
	class ClearTextCommand final : public GlCommand
	{
	public:
		TextureViewPtr m_texView;
		ClearValue m_val;

		ClearTextCommand(TextureViewPtr texView, const ClearValue& val)
			: m_texView(texView)
			, m_val(val)
		{
		}

		Error operator()(GlState&)
		{
			const TextureViewImpl& viewImpl = static_cast<TextureViewImpl&>(*m_texView);
			const TextureImpl& texImpl = static_cast<TextureImpl&>(*viewImpl.m_tex);

			texImpl.clear(viewImpl.getSubresource(), m_val);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(!self.m_state.insideRenderPass());
	self.pushBackNewCommand<ClearTextCommand>(texView, clearValue);
}

void CommandBuffer::fillBuffer(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
{
	class FillBufferCommand final : public GlCommand
	{
	public:
		BufferPtr m_buff;
		PtrSize m_offset;
		PtrSize m_size;
		U32 m_value;

		FillBufferCommand(BufferPtr buff, PtrSize offset, PtrSize size, U32 value)
			: m_buff(buff)
			, m_offset(offset)
			, m_size(size)
			, m_value(value)
		{
		}

		Error operator()(GlState&)
		{
			static_cast<BufferImpl&>(*m_buff).fill(m_offset, m_size, m_value);
			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(!self.m_state.insideRenderPass());
	self.pushBackNewCommand<FillBufferCommand>(buff, offset, size, value);
}

void CommandBuffer::writeOcclusionQueryResultToBuffer(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff)
{
	class WriteOcclResultToBuff final : public GlCommand
	{
	public:
		OcclusionQueryPtr m_query;
		PtrSize m_offset;
		BufferPtr m_buff;

		WriteOcclResultToBuff(OcclusionQueryPtr query, PtrSize offset, BufferPtr buff)
			: m_query(query)
			, m_offset(offset)
			, m_buff(buff)
		{
			ANKI_ASSERT((m_offset % 4) == 0);
		}

		Error operator()(GlState&)
		{
			const BufferImpl& buff = static_cast<const BufferImpl&>(*m_buff);
			ANKI_ASSERT(m_offset + 4 <= buff.getSize());

			glBindBuffer(GL_QUERY_BUFFER, buff.getGlName());
			glGetQueryObjectuiv(static_cast<const OcclusionQueryImpl&>(*m_query).getGlName(), GL_QUERY_RESULT,
								numberToPtr<GLuint*>(m_offset));
			glBindBuffer(GL_QUERY_BUFFER, 0);

			return Error::NONE;
		}
	};

	ANKI_GL_SELF(CommandBufferImpl);
	ANKI_ASSERT(!self.m_state.insideRenderPass());
	self.pushBackNewCommand<WriteOcclResultToBuff>(query, offset, buff);
}

void CommandBuffer::setPushConstants(const void* data, U32 dataSize)
{
	class PushConstants final : public GlCommand
	{
	public:
		DynamicArrayAuto<Vec4> m_data;

		PushConstants(const void* data, U32 dataSize, const CommandBufferAllocator<F32>& alloc)
			: m_data(alloc)
		{
			m_data.create(dataSize / sizeof(Vec4));
			memcpy(&m_data[0], data, dataSize);
		}

		Error operator()(GlState& state)
		{
			const ShaderProgramImplReflection& refl =
				static_cast<ShaderProgramImpl&>(*state.m_crntProg).getReflection();
			ANKI_ASSERT(refl.m_uniformDataSize == m_data.getSizeInBytes());

			const Bool transpose = true;
			for(const ShaderProgramImplReflection::Uniform& uni : refl.m_uniforms)
			{
				const U8* data = reinterpret_cast<const U8*>(&m_data[0]) + uni.m_pushConstantOffset;
				const U count = uni.m_arrSize;
				const GLint loc = uni.m_location;

				switch(uni.m_type)
				{
				case ShaderVariableDataType::VEC4:
					glUniform4fv(loc, count, reinterpret_cast<const GLfloat*>(data));
					break;
				case ShaderVariableDataType::IVEC4:
					glUniform4iv(loc, count, reinterpret_cast<const GLint*>(data));
					break;
				case ShaderVariableDataType::UVEC4:
					glUniform4uiv(loc, count, reinterpret_cast<const GLuint*>(data));
					break;
				case ShaderVariableDataType::MAT4:
					glUniformMatrix4fv(loc, count, transpose, reinterpret_cast<const GLfloat*>(data));
					break;
				case ShaderVariableDataType::MAT3:
				{
					// Remove the padding
					ANKI_ASSERT(count == 1 && "TODO");
					const Mat3x4* m34 = reinterpret_cast<const Mat3x4*>(data);
					Mat3 m3(m34->getRotationPart());
					glUniformMatrix3fv(loc, count, transpose, reinterpret_cast<const GLfloat*>(&m3));
					break;
				}
				default:
					ANKI_ASSERT(!"TODO");
				}
			}

			return Error::NONE;
		}
	};

	ANKI_ASSERT(data);
	ANKI_ASSERT(dataSize);
	ANKI_ASSERT(dataSize % 16 == 0);

	ANKI_GL_SELF(CommandBufferImpl);
	self.pushBackNewCommand<PushConstants>(data, dataSize, self.m_alloc);
}

void CommandBuffer::setRasterizationOrder(RasterizationOrder order)
{
	// Nothing for GL
}

} // end namespace anki
