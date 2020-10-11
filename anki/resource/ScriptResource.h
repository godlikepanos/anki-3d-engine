// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/ResourceObject.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// Script resource.
class ScriptResource : public ResourceObject
{
public:
	ScriptResource(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~ScriptResource();

	ANKI_USE_RESULT Error load(const ResourceFilename& filename, Bool async);

	CString getSource() const
	{
		return m_source.toCString();
	}

private:
	String m_source;
};
/// @}

} // namespace anki
