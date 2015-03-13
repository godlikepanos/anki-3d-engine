// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

/// @file
/// This file contains the public OpenGL headers and all the GL classes that
/// the rest of AnKi should use

#ifndef ANKI_GL_H
#define ANKI_GL_H

/// @defgroup opengl OpenGL
/// @ingroup anki_private

/// @defgroup opengl_private OpenGL private interfaces
/// Other modules should not use them.
/// @ingroup opengl

/// @defgroup opengl_containers OpenGL container objects
/// The containers of OpenGL
/// @ingroup opengl

/// @defgroup opengl_other OpenGL uncategorized interfaces 
/// @ingroup opengl

#include "anki/gr/GlBufferHandle.h"
#include "anki/gr/GlTextureHandle.h"
#include "anki/gr/GlShaderHandle.h"

#include "anki/gr/GlFramebufferHandle.h"
#include "anki/gr/GlPipelineHandle.h"

#include "anki/gr/GlCommandBufferHandle.h"
#include "anki/gr/GlOcclusionQueryHandle.h"
#include "anki/gr/GlDevice.h"

#endif
