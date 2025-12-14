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

Error SceneSerializer::read([[maybe_unused]] CString name, [[maybe_unused]] WeakArray<F64> values)
{
	ANKI_ASSERT(!"TODO");
	return Error::kNone;
}

} // end namespace anki
