// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_OCCLUSION_QUERY_HANDLE_H
#define ANKI_GR_OCCLUSION_QUERY_HANDLE_H

#include "anki/gr/GrHandle.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Occlusion query handle.
class OcclusionQueryHandle: public GrHandle<OcclusionQueryImpl>
{
public:
	using Base = GrHandle<OcclusionQueryImpl>;
	using ResultBit = OcclusionQueryResultBit;

	OcclusionQueryHandle();

	~OcclusionQueryHandle();

	/// Create a query.
	ANKI_USE_RESULT Error create(
		GrManager* manager, ResultBit condRenderingBit);

	/// Begin query.
	void begin(CommandBufferHandle& commands);

	/// End query.
	void end(CommandBufferHandle& commands);
};
/// @}

} // end namespace anki

#endif

