#ifndef ANKI_GL_CONTEXT_NON_SHARABLE_H
#define ANKI_GL_CONTEXT_NON_SHARABLE_H

#include "anki/Config.h"
#include "anki/util/Assert.h"
#include <thread>

namespace anki {

/// @addtogroup OpenGL
/// @{

/// Defines an non sharable GL object. Used to avoid idiotic mistakes and more
/// specifically using the object from other than contexts
class ContextNonSharable
{
public:
	ContextNonSharable()
	{}

	ContextNonSharable(const ContextNonSharable& b)
#if ANKI_DEBUG
		: creationThreadId(b.creationThreadId)
#endif
	{}

	ContextNonSharable(ContextNonSharable&& b)
#if ANKI_DEBUG
		: creationThreadId(b.creationThreadId)
#endif
	{}

	~ContextNonSharable()
	{
		checkNonSharable();
	}

	ContextNonSharable& operator=(const ContextNonSharable& b)
	{
#if ANKI_DEBUG
		creationThreadId = b.creationThreadId;
#endif
		return *this;
	}

	ContextNonSharable& operator=(ContextNonSharable&& b)
	{
#if ANKI_DEBUG
		creationThreadId = b.creationThreadId;
#endif
		return *this;
	}

	void crateNonSharable()
	{
#if ANKI_DEBUG
		creationThreadId = std::this_thread::get_id();
#endif
	}

	void checkNonSharable() const
	{
#if ANKI_DEBUG
		ANKI_ASSERT(creationThreadId == std::this_thread::get_id() && 
				"Object is not context sharable");
#endif
	}

private:
#if ANKI_DEBUG
	std::thread::id creationThreadId;
#endif
};
/// @}

} // end namespace anki

#endif
