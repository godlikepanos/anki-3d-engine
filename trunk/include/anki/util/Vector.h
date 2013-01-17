#ifndef ANKI_UTIL_VECTOR_H
#define ANKI_UTIL_VECTOR_H

#include "anki/util/Assert.h"
#include "anki/util/Functions.h"
#include "anki/util/Allocator.h"
#include "anki/util/Memory.h"
#include <vector>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup containers
/// @{

template<typename T, typename Alloc = Allocator<T>>
using Vector = std::vector<T, Alloc>;

/// Vector of pointers. The same as the regular vector but it deallocates the
/// pointers
template<typename T, typename Alloc = Allocator<T>>
class PtrVector: public Vector<T*, Alloc>
{
public:
	typedef Vector<T*, Alloc> Base;

	~PtrVector()
	{
		for(typename Base::iterator it = Base::begin(); it != Base::end(); it++)
		{
			propperDelete(*it);
		}
	}

	typename Base::iterator erase(typename Base::iterator pos)
	{
		return Base::erase(pos);
	}
};

/// @}
/// @}

} // end namespace anki

#endif
