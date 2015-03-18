// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_OCCLUSION_QUERY_IMPL_H
#define ANKI_GR_GL_OCCLUSION_QUERY_IMPL_H

#include "anki/gr/gl/GlObject.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Occlusion query.
class OcclusionQueryImpl: public GlObject
{
public:
	using Base = GlObject;
	using ResultBit = OcclusionQueryResultBit;
	using Result = OcclusionQueryResult;

	OcclusionQueryImpl(GrManager* manager)
	:	Base(manager)
	{}

	~OcclusionQueryImpl()
	{
		destroy();
	}

	/// Create the query.
	/// @param condRenderingBit If the query is used in conditional rendering
	///        the result will be checked against this mask. If the result
	///        contains any of the bits then the dracall will not be skipped.
	ANKI_USE_RESULT Error create(ResultBit condRenderingBit);

	/// Begin query.
	void begin();

	/// End query.
	void end();

	/// Get query result.
	Result getResult() const;

	/// Return true if the drawcall should be skipped.
	Bool skipDrawcall() const
	{
		U resultBit = 1 << U(getResult());
		U condBit = U(m_condRenderingBit);
		return !(resultBit & condBit);
	}

private:
	ResultBit m_condRenderingBit;
	void destroy();
};
/// @}

} // end namespace anki

#endif

