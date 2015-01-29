// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_SCRIPT_H
#define ANKI_RESOURCE_SCRIPT_H

#include "anki/resource/Common.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Script resource.
class Script
{
public:
	Script(ResourceAllocator<U8>&)
	{}

	~Script();

	ANKI_USE_RESULT Error load(
		const CString& filename, ResourceInitializer& init);

	CString getSource() const
	{
		return m_source.toCString();
	}

private:
	ResourceString m_source;
	ResourceAllocator<char> m_alloc;
};
/// @}

} // end namespace

#endif
