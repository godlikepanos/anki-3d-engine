// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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

ShaderProgram::ShaderProgram(GrManager* manager, U64 hash, GrObjectCache* cache)
	: GrObject(manager, CLASS_TYPE, hash, cache)
{
}

ShaderProgram::~ShaderProgram()
{
}

void ShaderProgram::init(ShaderPtr vert, ShaderPtr frag)
{
	class CreateCommand final : public GlCommand
	{
	public:
		ShaderProgramPtr m_prog;
		ShaderPtr m_vert;
		ShaderPtr m_frag;

		CreateCommand(ShaderProgram* prog, ShaderPtr vert, ShaderPtr frag)
			: m_prog(prog)
			, m_vert(vert)
			, m_frag(frag)
		{
		}

		Error operator()(GlState&)
		{
			ShaderPtr none;
			return m_prog->m_impl->initGraphics(m_vert, none, none, none, m_frag);
		}
	};

	m_impl.reset(getAllocator().newInstance<ShaderProgramImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());
	cmdb->m_impl->pushBackNewCommand<CreateCommand>(this, vert, frag);
	cmdb->flush();
}

void ShaderProgram::init(ShaderPtr comp)
{
	class CreateCommand final : public GlCommand
	{
	public:
		ShaderProgramPtr m_prog;
		ShaderPtr m_comp;

		CreateCommand(ShaderProgram* prog, ShaderPtr comp)
			: m_prog(prog)
			, m_comp(comp)
		{
		}

		Error operator()(GlState&)
		{
			ShaderPtr none;
			return m_prog->m_impl->initCompute(m_comp);
		}
	};

	m_impl.reset(getAllocator().newInstance<ShaderProgramImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());
	cmdb->m_impl->pushBackNewCommand<CreateCommand>(this, comp);
	cmdb->flush();
}

void ShaderProgram::init(ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag)
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

		CreateCommand(
			ShaderProgram* prog, ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag)
			: m_prog(prog)
			, m_vert(vert)
			, m_tessc(tesse)
			, m_tesse(tesse)
			, m_geom(geom)
			, m_frag(frag)
		{
		}

		Error operator()(GlState&)
		{
			return m_prog->m_impl->initGraphics(m_vert, m_tessc, m_tesse, m_geom, m_frag);
		}
	};

	m_impl.reset(getAllocator().newInstance<ShaderProgramImpl>(&getManager()));

	CommandBufferPtr cmdb = getManager().newInstance<CommandBuffer>(CommandBufferInitInfo());
	cmdb->m_impl->pushBackNewCommand<CreateCommand>(this, vert, tessc, tesse, geom, frag);
	cmdb->flush();
}

} // end namespace anki
