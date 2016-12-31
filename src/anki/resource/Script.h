// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class Script : public ResourceObject
{
public:
	Script(ResourceManager* manager)
		: ResourceObject(manager)
	{
	}

	~Script();

	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	CString getSource() const
	{
		return m_source.toCString();
	}

private:
	String m_source;
};
/// @}

} // end namespace
