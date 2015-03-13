// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_SHADER_HANDLE_H
#define ANKI_GL_GL_SHADER_HANDLE_H

#include "anki/gr/GlContainerHandle.h"

namespace anki {

/// @addtogroup opengl_containers
/// @{

/// Program handle
class ShaderHandle: public GlContainerHandle<ShaderImpl>
{
public:
	using Base = GlContainerHandle<ShaderImpl>;

	ShaderHandle();

	~ShaderHandle();

	/// Create shader program.
	ANKI_USE_RESULT Error create(CommandBufferHandle& commands, 
		GLenum shaderType, const ClientBufferHandle& source);

	/// @name Accessors
	/// They will sync client with server.
	/// @{
	GLenum getType() const;
	/// @}
};

/// @}

} // end namespace anki

#endif

