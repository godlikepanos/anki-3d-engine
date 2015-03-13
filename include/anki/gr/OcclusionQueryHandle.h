// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_OCCLUSION_QUERY_HANDLE_H
#define ANKI_GR_OCCLUSION_QUERY_HANDLE_H

#include "anki/gr/GlContainerHandle.h"

namespace anki {

/// @addtogroup opengl_other
/// @{

/// Occlusion query handle.
class OcclusionQueryHandle: public GlContainerHandle<OcclusionQueryImpl>
{
public:
	using Base = GlContainerHandle<OcclusionQueryImpl>;
	using ResultBit = GlOcclusionQueryResultBit;

	OcclusionQueryHandle();

	~OcclusionQueryHandle();

	/// Create a query.
	ANKI_USE_RESULT Error create(GlDevice* dev, ResultBit condRenderingBit);

	/// Begin query.
	void begin(CommandBufferHandle& commands);

	/// End query.
	void end(CommandBufferHandle& commands);
};
/// @}

} // end namespace anki

#endif

