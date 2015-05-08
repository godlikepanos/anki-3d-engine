// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_SCRIPT_H
#define ANKI_RESOURCE_SCRIPT_H

#include "anki/resource/ResourceObject.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Script resource.
class Script: public ResourceObject
{
public:
	Script(ResourceManager* manager)
	:	ResourceObject(manager)
	{}

	~Script();

	ANKI_USE_RESULT Error load(const CString& filename);

	CString getSource() const
	{
		return m_source.toCString();
	}

private:
	String m_source;
};
/// @}

} // end namespace

#endif
