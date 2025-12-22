// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/SceneSerializer.h>

namespace anki {

Error SceneSerializer::write(CString name, ConstWeakArray<F64> values)
{
	Array<F32, 32> tmpArray;
	WeakArray<F32> arr;
	if(values.getSize() < tmpArray.getSize())
	{
		arr = {tmpArray.getBegin(), values.getSize()};
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	for(U32 i = 0; i < values.getSize(); ++i)
	{
		arr[i] = F32(values[i]);
	}

	return write(name, arr);
}

Error SceneSerializer::read(CString name, WeakArray<F64> values)
{
	Array<F32, 32> tmpArray;
	WeakArray<F32> arr;
	if(values.getSize() < tmpArray.getSize())
	{
		arr = {tmpArray.getBegin(), values.getSize()};
	}
	else
	{
		ANKI_ASSERT(!"TODO");
	}

	return read(name, arr);
}

Error TextSceneSerializer::parseCurrentLine(SceneStringList& tokens, CString fieldName, U32 checkTokenCount)
{
	if(m_read.m_linesIt == m_read.m_lines.getEnd())
	{
		ANKI_SERIALIZER_LOGE("Can't read next line");
		return Error::kUserData;
	}

	const SceneString& line = *m_read.m_linesIt;
	tokens.splitString(line, ' ');
	if(tokens.getSize() == 0)
	{
		ANKI_SERIALIZER_LOGE("Line empty");
		return Error::kUserData;
	}

	if(*tokens.getBegin() != fieldName)
	{
		ANKI_SERIALIZER_LOGE("Wrong token. Got: %s, expecting: %s", tokens.getBegin()->cstr(), fieldName.cstr());
		return Error::kUserData;
	}

	tokens.popFront();

	if(checkTokenCount < kMaxU32 && tokens.getSize() != checkTokenCount)
	{
		ANKI_SERIALIZER_LOGE("Incorrect number of tokens");
		return Error::kUserData;
	}

	++m_read.m_linesIt;
	++m_read.m_lineno;

	return Error::kNone;
}

} // end namespace anki
