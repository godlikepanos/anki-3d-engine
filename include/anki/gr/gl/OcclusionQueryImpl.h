// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// Occlusion query.
class OcclusionQueryImpl : public GlObject
{
public:
	OcclusionQueryImpl(GrManager* manager)
		: GlObject(manager)
	{
	}

	~OcclusionQueryImpl()
	{
		destroyDeferred(glDeleteQueries);
	}

	/// Create the query.
	/// @param condRenderingBit If the query is used in conditional rendering
	///        the result will be checked against this mask. If the result
	///        contains any of the bits then the dracall will not be skipped.
	void create(OcclusionQueryResultBit condRenderingBit);

	/// Begin query.
	void begin();

	/// End query.
	void end();

	/// Get query result.
	OcclusionQueryResult getResult() const;

	/// Return true if the drawcall should be skipped.
	Bool skipDrawcall() const
	{
		U resultBit = 1 << U(getResult());
		U condBit = U(m_condRenderingBit);
		return !(resultBit & condBit);
	}

private:
	OcclusionQueryResultBit m_condRenderingBit;
};
/// @}

} // end namespace anki
