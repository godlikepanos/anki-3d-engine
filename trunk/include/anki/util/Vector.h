#ifndef ANKI_UTIL_VECTOR_H
#define ANKI_UTIL_VECTOR_H

#include "anki/util/Assert.h"
#include <vector>

namespace anki {

/// @addtogroup util
/// @{

template<typename T>
using Vector = std::vector<T>;

template<typename T>
class PtrVector: public Vector<T*>
{
public:
	~PtrVector()
	{
		for(typename Vector<T*>::iterator it = Vector<T*>::begin();
			it != Vector<T*>::end(); it++)
		{
			typedef char TypeMustBeComplete[sizeof(T) ? 1 : -1];
    		(void) sizeof(TypeMustBeComplete);
			delete *it;
		}
	}
};

} // end namespace anki

#endif
