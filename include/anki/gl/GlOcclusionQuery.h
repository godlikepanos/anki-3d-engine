// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GL_GL_OCCLUSION_QUERY_H
#define ANKI_GL_GL_OCCLUSION_QUERY_H

#include "anki/gl/GlObject.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// Occlusion query.
class GlOcclusionQuery: public GlObject
{
public:
	using Base = GlObject;

	enum class Result: U
	{
		NOT_AVAILABLE,
		VISIBLE,
		NOT_VISIBLE
	};

	GlOcclusionQuery() = default;

	~GlOcclusionQuery()
	{
		destroy();
	}

	/// Create the query.
	ANKI_USE_RESULT Error create();

	/// Begin query.
	void begin();

	/// End query.
	void end();

	/// Get query result.
	Result getResult();

private:
	void destroy();
};
/// @}

} // end namespace anki

#endif

