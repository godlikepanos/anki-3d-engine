// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_SCOPE_DESTROYER_H
#define ANKI_UTIL_SCOPE_DESTROYER_H

namespace anki {

/// @addtogroup util_containers
/// @{

/// A class that destroys other instances when it get out of scope.
template<typename TInstance, typename TAlloc>
class ScopeDestroyer
{
public:
	TInstance* m_inst;
	TAlloc m_alloc;

	ScopeDestroyer(TInstance* inst, TAlloc alloc)
	:	m_inst(inst),
		m_alloc(alloc)
	{}

	~ScopeDestroyer()
	{
		m_inst->destroy(m_alloc);
	}
};

/// @}

} // end namespace anki

#endif
