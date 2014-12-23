// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_OCCLUSION_QUERY_HANDLE_H
#define ANKI_GL_GL_OCCLUSION_QUERY_HANDLE_H

#include "anki/gl/GlContainerHandle.h"

namespace anki {

// Forward
class GlOcclusionQuery;

/// @addtogroup opengl_other
/// @{

/// Occlusion query handle.
class GlOcclusionQueryHandle: public GlContainerHandle<GlOcclusionQuery>
{
public:
	using Base = GlContainerHandle<GlOcclusionQuery>;
	using ResultBit = GlOcclusionQueryResultBit;

	GlOcclusionQueryHandle();

	~GlOcclusionQueryHandle();

	/// Create a query.
	ANKI_USE_RESULT Error create(GlDevice* dev, ResultBit condRenderingBit);

	/// Begin query.
	void begin(GlCommandBufferHandle& commands);

	/// End query.
	void end(GlCommandBufferHandle& commands);
};
/// @}

} // end namespace anki

#endif

