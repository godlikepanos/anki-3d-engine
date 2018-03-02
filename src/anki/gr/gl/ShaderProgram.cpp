// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ShaderProgram.h>
#include <anki/gr/gl/ShaderProgramImpl.h>
#include <anki/gr/gl/CommandBufferImpl.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/Shader.h>

namespace anki
{

ShaderProgram* ShaderProgram::newInstance(GrManager* manager, const ShaderProgramInitInfo& init)
{
	class CreateCommand final : public GlCommand
	{
	public:
		ShaderProgramPtr m_prog;
		ShaderPtr m_vert;
		ShaderPtr m_tessc;
		ShaderPtr m_tesse;
		ShaderPtr m_geom;
		ShaderPtr m_frag;
		ShaderPtr m_comp;

		CreateCommand(ShaderProgram* prog,
			ShaderPtr vert,
			ShaderPtr tessc,
			ShaderPtr tesse,
			ShaderPtr geom,
			ShaderPtr frag,
			ShaderPtr comp)
			: m_prog(prog)
			, m_vert(vert)
			, m_tessc(tessc)
			, m_tesse(tesse)
			, m_geom(geom)
			, m_frag(frag)
			, m_comp(comp)
		{
		}

		Error operator()(GlState&)
		{
			if(m_comp)
			{
				return static_cast<ShaderProgramImpl&>(*m_prog).initCompute(m_comp);
			}
			else
			{
				return static_cast<ShaderProgramImpl&>(*m_prog).initGraphics(m_vert, m_tessc, m_tesse, m_geom, m_frag);
			}
		}
	};

	ANKI_ASSERT(init.isValid());

	ShaderProgramImpl* impl = manager->getAllocator().newInstance<ShaderProgramImpl>(manager, init.getName());
	impl->getRefcount().fetchAdd(1); // Hold a reference in case the command finishes and deletes quickly

	CommandBufferPtr cmdb = manager->newCommandBuffer(CommandBufferInitInfo());
	static_cast<CommandBufferImpl&>(*cmdb).pushBackNewCommand<CreateCommand>(impl,
		init.m_shaders[ShaderType::VERTEX],
		init.m_shaders[ShaderType::TESSELLATION_CONTROL],
		init.m_shaders[ShaderType::TESSELLATION_EVALUATION],
		init.m_shaders[ShaderType::GEOMETRY],
		init.m_shaders[ShaderType::FRAGMENT],
		init.m_shaders[ShaderType::COMPUTE]);
	static_cast<CommandBufferImpl&>(*cmdb).flush();

	return impl;
}

} // end namespace anki
