// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/resource/Script.h"
#include "anki/util/File.h"

namespace anki {

//==============================================================================
Script::~Script()
{
	m_source.destroy(getAllocator());
}

//==============================================================================
Error Script::load(const CString& filename)
{
	File file;

	ANKI_CHECK(file.open(filename, File::OpenFlag::READ));
	ANKI_CHECK(file.readAllText(getAllocator(), m_source));

	return ErrorCode::NONE;
}

} // end namespace anki
