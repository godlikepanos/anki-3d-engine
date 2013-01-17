#ifndef ANKI_GL_GL_OBJECT_H
#define ANKI_GL_GL_OBJECT_H

#include "anki/gl/Ogl.h"
#include "anki/util/NonCopyable.h"
#include "anki/util/Assert.h"
#include "anki/util/StdTypes.h"
#include "anki/Config.h"
#include <thread>

namespace anki {

/// @addtogroup OpenGL
/// @{

/// An OpenGL object. It is never copyable
class GlObject: public NonCopyable
{
public:
	/// Default
	GlObject()
	{}

	/// Move
	GlObject(GlObject&& b)
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
		ANKI_DEBUG(!isCreated());
		glId = b.glId;
		b.glId = 0;
		return *this;
	}

	/// Get the GL name
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
	GLuint glId = 0;
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
