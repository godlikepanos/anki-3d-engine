// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>

namespace anki {

class PipelineQueryInitInfo : public GrBaseInitInfo
{
public:
	PipelineQueryType m_type = PipelineQueryType::kCount;

	using GrBaseInitInfo::GrBaseInitInfo;
};

// Query of various pipeline statistics.
class PipelineQuery : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kPipelineQuery;

	// Get the occlusion query result. It won't block.
	PipelineQueryResult getResult(U64& value) const;

protected:
	PipelineQuery(CString name)
		: GrObject(kClassType, name)
	{
	}

	~PipelineQuery()
	{
	}

private:
	[[nodiscard]] static PipelineQuery* newInstance(const PipelineQueryInitInfo& inf);
};

} // end namespace anki
