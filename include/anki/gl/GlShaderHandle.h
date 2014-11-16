// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_SHADER_HANDLE_H
#define ANKI_GL_GL_SHADER_HANDLE_H

#include "anki/gl/GlContainerHandle.h"

namespace anki {

// Forward
class GlShader;
class GlClientBufferHandle;

/// @addtogroup opengl_containers
/// @{

/// Program handle
class GlShaderHandle: public GlContainerHandle<GlShader>
{
public:
	using Base = GlContainerHandle<GlShader>;

	GlShaderHandle();

	~GlShaderHandle();

	/// Create shader program.
	ANKI_USE_RESULT Error create(GlCommandBufferHandle& commands, 
		GLenum shaderType, const GlClientBufferHandle& source);

	/// @name Accessors
	/// They will sync client with server.
	/// @{
	GLenum getType() const;

	const GlProgramVariable* getVariablesBegin() const;
	const GlProgramVariable* getVariablesEnd() const;

	const GlProgramBlock* getBlocksBegin() const;
	const GlProgramBlock* getBlocksEnd() const;

	const GlProgramVariable* tryFindVariable(const CString& name) const;

	const GlProgramBlock* tryFindBlock(const CString& name) const;
	/// @}
};

/// @}

} // end namespace anki

#endif

