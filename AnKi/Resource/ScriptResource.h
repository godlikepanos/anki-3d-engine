// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Resource/ResourceObject.h>

namespace anki {

/// @addtogroup resource
/// @{

/// Script resource.
class ScriptResource : public ResourceObject
{
public:
	ScriptResource() = default;

	~ScriptResource() = default;

	Error load(const ResourceFilename& filename, Bool async);

	CString getSource() const
	{
		return m_source.toCString();
	}

private:
	ResourceString m_source;
};
/// @}

} // namespace anki
