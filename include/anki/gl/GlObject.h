#ifndef ANKI_GL_GL_OBJECT_H
#define ANKI_GL_GL_OBJECT_H

#include "anki/gl/GlCommon.h"

namespace anki {

/// @addtogroup opengl_private
/// @{

/// A GL object
class GlObject: public NonCopyable
{
public:
	/// Default
	GlObject()
		: m_glName(0)
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
		
		m_glName = b.m_glName;
		b.m_glName = 0;
		return *this;
	}

	/// Get the GL name for the current frame
	GLuint getGlName() const
	{
		ANKI_ASSERT(isCreated());
		return m_glName;
	}

	/// GL object is created
	Bool isCreated() const
	{
		return m_glName != 0;
	}

protected:
	GLuint m_glName; ///< OpenGL name
};

/// @}

} // end namespace anki

#endif
