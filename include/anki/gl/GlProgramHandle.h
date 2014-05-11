#ifndef ANKI_GL_GL_PROGRAM_HANDLE_H
#define ANKI_GL_GL_PROGRAM_HANDLE_H

#include "anki/gl/GlContainerHandle.h"
#include "anki/util/Vector.h"

namespace anki {

// Forward
class GlProgram;
class GlClientBufferHandle;

/// @addtogroup opengl_containers
/// @{

/// Program handle
class GlProgramHandle: public GlContainerHandle<GlProgram>
{
public:
	typedef GlContainerHandle<GlProgram> Base;

	// Re-define it here
	template<typename T>
	using ProgramVector = Vector<T, GlGlobalHeapAllocator<T>>;

	/// @name Contructors/Destructor
	/// @{
	GlProgramHandle();

	/// Create program
	explicit GlProgramHandle(GlJobChainHandle& jobs, 
		GLenum shaderType, const GlClientBufferHandle& source);

	~GlProgramHandle();
	/// @}

	/// @name Accessors
	/// They will sync
	/// @{
	GLenum getType() const;

	const ProgramVector<GlProgramVariable>& getVariables() const;

	const ProgramVector<GlProgramBlock>& getBlocks() const;

	const GlProgramVariable& findVariable(const char* name) const;

	const GlProgramBlock& findBlock(const char* name) const;

	const GlProgramVariable* tryFindVariable(const char* name) const;

	const GlProgramBlock* tryFindBlock(const char* name) const;
	/// @}
};

/// @}

} // end namespace anki

#endif

