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
	m_source.destroy(m_alloc);
}

//==============================================================================
Error Script::load(const CString& filename, ResourceInitializer& init)
{
	Error err = ErrorCode::NONE;
	File file;

	err = file.open(filename, File::OpenFlag::READ);

	if(!err)
	{
		m_alloc = init.m_alloc;
		err = file.readAllText(m_alloc, m_source);
	}

	return err;
}

} // end namespace anki
