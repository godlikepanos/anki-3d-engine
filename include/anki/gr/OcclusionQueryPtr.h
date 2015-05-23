// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_OCCLUSION_QUERY_HANDLE_H
#define ANKI_GR_OCCLUSION_QUERY_HANDLE_H

#include "anki/gr/GrPtr.h"

namespace anki {

/// @addtogroup graphics
/// @{

/// Occlusion query.
class OcclusionQueryPtr: public GrPtr<OcclusionQueryImpl>
{
public:
	using Base = GrPtr<OcclusionQueryImpl>;
	using ResultBit = OcclusionQueryResultBit;

	OcclusionQueryPtr();

	~OcclusionQueryPtr();

	/// Create a query.
	void create(GrManager* manager, ResultBit condRenderingBit);

	/// Begin query.
	void begin(CommandBufferPtr& commands);

	/// End query.
	void end(CommandBufferPtr& commands);
};
/// @}

} // end namespace anki

#endif

