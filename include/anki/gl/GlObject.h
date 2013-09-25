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
	/// Buffering technique
	enum
	{
		SINGLE_OBJECT = 1,
		DOUBLE_OBJECT = 2,
		TRIPLE_OBJECT = 3,
		MAX_OBJECTS = 3
	};

	/// Default
	GlObject()
	{
		memset(&glIds[0], 0, sizeof(glIds));
		objectsCount = 1;
#if ANKI_DEBUG
		refCount.store(0);
#endif
	}

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
		
		for(U i = 0; i < MAX_OBJECTS; i++)
		{
			glIds[i] = b.glIds[i];
			b.glIds[i] = 0;
		}

		objectsCount = b.objectsCount;
		b.objectsCount = 1;
#if ANKI_DEBUG
		refCount.store(b.refCount.load());
		b.refCount.store(0);
#endif
		return *this;
	}

	/// Get the GL name for the current frame
	GLuint getGlId() const
	{
		ANKI_ASSERT(isCreated());
		return glIds[getGlobTimestamp() % objectsCount];
	}

	/// GL object is created
	Bool isCreated() const
	{
		ANKI_ASSERT(objectsCount > 0);
#if ANKI_DEBUG
		U mask = 0;
		for(U i = 0; i < MAX_OBJECTS; i++)
		{
			mask <<= 1;
			mask |= (glIds[i] != 0);
		}

		// If the mask is not zero then make sure that objectsCount is sane
		ANKI_ASSERT(!(mask != 0 && __builtin_popcount(mask) != objectsCount));
#endif

		return glId != 0;
	}

protected:
	/// OpenGL names
	union
	{
		Array<GLuint, MAX_OBJECTS> glIds;
		GLuint glId;
	};

	/// The size of the glIds array
	U8 objectsCount;

#if ANKI_DEBUG
	/// Textures and buffers can be attached so keep a refcount for sanity
	/// checks
	std::atomic<U32> refCount;
#endif
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
