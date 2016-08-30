// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/CommandBuffer.h>
#include <anki/util/Assert.h>
#include <anki/util/Allocator.h>

namespace anki
{

// Forward
class GlState;

/// @addtogroup opengl
/// @{

template<typename T>
using CommandBufferAllocator = StackAllocator<T>;

/// The base of all GL commands.
class GlCommand
{
public:
	GlCommand* m_nextCommand = nullptr;

	virtual ~GlCommand()
	{
	}

	/// Execute command
	virtual ANKI_USE_RESULT Error operator()(GlState& state) = 0;
};

/// A number of GL commands organized in a chain
class CommandBufferImpl
{
public:
	using InitHints = CommandBufferInitHints;

#if ANKI_ASSERTS_ENABLED
	class StateSet
	{
	public:
		Bool m_viewport = false;
		Bool m_polygonOffset = false;
		Bool m_insideRenderPass = false;
		Bool m_secondLevel = false;
	} m_dbg;
#endif

	/// Default constructor
	CommandBufferImpl(GrManager* manager)
		: m_manager(manager)
	{
	}

	~CommandBufferImpl()
	{
		destroy();
	}

	/// Initialize.
	void init(const CommandBufferInitInfo& init);

	/// Get the internal allocator.
	CommandBufferAllocator<U8> getInternalAllocator() const
	{
		return m_alloc;
	}

	/// For the UniquePtr destructor.
	GrAllocator<U8> getAllocator() const;

	/// Compute initialization hints.
	InitHints computeInitHints() const;

	/// Create a new command and add it to the chain.
	template<typename TCommand, typename... TArgs>
	void pushBackNewCommand(TArgs&&... args);

	/// Execute all commands
	ANKI_USE_RESULT Error executeAllCommands();

	/// Fake that it's been executed
	void makeExecuted()
	{
#if ANKI_DEBUG
		m_executed = true;
#endif
	}

	/// Make immutable
	void makeImmutable()
	{
		m_immutable = true;
	}

	GrManager& getManager()
	{
		return *m_manager;
	}

	Bool isEmpty() const
	{
		return m_firstCommand == nullptr;
	}

	void bindResourceGroup(ResourceGroupPtr rc, U slot, const TransientMemoryInfo* info);

	void drawElements(U32 count, U32 instanceCount = 1, U32 firstIndex = 0, U32 baseVertex = 0, U32 baseInstance = 0);

	void drawArrays(U32 count, U32 instanceCount = 1, U32 first = 0, U32 baseInstance = 0);

	void drawElementsConditional(OcclusionQueryPtr query,
		U32 count,
		U32 instanceCount = 1,
		U32 firstIndex = 0,
		U32 baseVertex = 0,
		U32 baseInstance = 0);

	void drawArraysConditional(
		OcclusionQueryPtr query, U32 count, U32 instanceCount = 1, U32 first = 0, U32 baseInstance = 0);

	void dispatchCompute(U32 groupCountX, U32 groupCountY, U32 groupCountZ);

private:
	GrManager* m_manager = nullptr;
	GlCommand* m_firstCommand = nullptr;
	GlCommand* m_lastCommand = nullptr;
	CommandBufferAllocator<U8> m_alloc;
	Bool8 m_immutable = false;

#if ANKI_DEBUG
	Bool8 m_executed = false;
#endif

	void destroy();

	void checkDrawcall() const
	{
		ANKI_ASSERT(m_dbg.m_viewport == true);
		ANKI_ASSERT(m_dbg.m_polygonOffset == true);
	}
};

template<typename TCommand, typename... TArgs>
inline void CommandBufferImpl::pushBackNewCommand(TArgs&&... args)
{
	ANKI_ASSERT(!m_immutable);
	TCommand* newCommand = m_alloc.template newInstance<TCommand>(std::forward<TArgs>(args)...);

	if(m_firstCommand != nullptr)
	{
		ANKI_ASSERT(m_lastCommand != nullptr);
		ANKI_ASSERT(m_lastCommand->m_nextCommand == nullptr);
		m_lastCommand->m_nextCommand = newCommand;
		m_lastCommand = newCommand;
	}
	else
	{
		ANKI_ASSERT(m_lastCommand == nullptr);
		m_firstCommand = newCommand;
		m_lastCommand = newCommand;
	}
}
/// @}

} // end namespace anki
