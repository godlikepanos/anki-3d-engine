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
		ANKI_ASSERT(!isCreated());
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

/// A compound GL object that supports buffering depending on the frame
class GlMultiObject: public NonCopyable
{
public:
	/// Buffering technique
	enum
	{
		SINGLE_OBJECT = 1,
		DOUBLE_OBJECT = 2,
		TRIPLE_OBJECT = 3,
		MAX_OBJECTS = 3
	};

	/// Default
	GlMultiObject()
	{}

	/// Move
	GlMultiObject(GlMultiObject&& b)
	{
		*this = std::move(b);
	}

	~GlMultiObject()
	{
		// The destructor of the derived GL object should pass 0 name
		ANKI_ASSERT(!isCreated());
	}

	/// Move
	GlMultiObject& operator=(GlMultiObject&& b)
	{
		ANKI_ASSERT(!isCreated());
		
		for(U i = 0; i < MAX_OBJECTS; i++)
		{
			glId[i] = b.glId[i];
			b.glId[i] = 0;
		}

		objectsCount = b.objectsCount;
		b.objectsCount = 0;
		return *this;
	}

	/// Get the GL name
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glId[getGlobTimestamp() % MAX_OBJECTS];
	}

	/// GL object is created
	Bool isCreated() const
	{
#if ANKI_DEBUG
		U mask = 0;
		for(U i = 0; i < MAX_OBJECTS; i++)
		{
			mask <<= 1;
			mask |= (glId[i] != 0);
		}

		// If the mask is not zero then make sure that objectsCount is sane
		ANKI_ASSERT(!(mask != 0 && __builtin_popcount(mask) != objectsCount));
#endif

		return objectsCount > 0;
	}

protected:
	/// OpenGL names
	Array<GLuint, MAX_OBJECTS> glId = {{0, 0, 0}};

	/// The size of the glId array
	U8 objectsCount = 0;
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
