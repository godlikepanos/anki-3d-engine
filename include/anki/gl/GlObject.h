#ifndef ANKI_GL_GL_OBJECT_H
#define ANKI_GL_GL_OBJECT_H

#include "anki/gl/Ogl.h"
#include "anki/gl/GlException.h"
#include "anki/Config.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Assert.h"
#include "anki/util/Array.h"
#include "anki/util/StdTypes.h"
#include "anki/core/Timestamp.h"
#include <thread>
#include <atomic>
#include <cstring>

namespace anki {

/// @addtogroup OpenGL
/// @{

/// A compound GL object that supports buffering depending on the frame
class GlObject: public NonCopyable
{
public:
	/// Default
	GlObject()
		: glId(0)
	{}

	/// Move
	GlObject(GlObject&& b)
		: GlObject()
	{
		*this = std::move(b);
	}

	~GlObject()
	{
		// The destructor of the derived GL object should pass 0 name
		ANKI_ASSERT(!isCreated());
	}

	/// Move
	GlObject& operator=(GlObject&& b)
	{
		ANKI_ASSERT(!isCreated());
		
		glId = b.glId;
		b.glId = 0;
		return *this;
	}

	/// Get the GL name for the current frame
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glId;
	}

	/// GL object is created
	Bool isCreated() const
	{
		return glId != 0;
	}

protected:
	/// OpenGL name
	GLuint glId;
};

/// Defines an non sharable GL object. Used to avoid idiotic mistakes and more
/// specifically using the object from other than contexts
class GlObjectContextNonSharable: protected GlObject
{
public:
	GlObjectContextNonSharable()
	{}

	GlObjectContextNonSharable(GlObjectContextNonSharable&& b)
	{
		*this = std::move(b);
	}

	~GlObjectContextNonSharable()
	{
		checkNonSharable();
	}

	GlObjectContextNonSharable& operator=(GlObjectContextNonSharable&& b)
	{
		GlObject::operator=(std::forward<GlObject>(b));
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
		ANKI_ASSERT((!isCreated()
			|| creationThreadId == std::this_thread::get_id())
			&& "Object is not context sharable");
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
