// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/Common.h>

namespace anki
{

/// @addtogroup opengl
/// @{

/// State of the handle
enum class GlObjectState : U32
{
	NEW,
	TO_BE_CREATED,
	CREATED,
	TO_BE_DELETED,
	DELETED,
	ERROR
};

/// A GL object
class GlObject
{
public:
	using State = GlObjectState;
	using GlDeleteFunction = void (*)(GLsizei, const GLuint*);

	/// Default
	GlObject();

	~GlObject()
	{
		// The destructor of the derived GL object should pass 0 name
		ANKI_ASSERT(!isCreated());
	}

	/// Get the GL name for the current frame
	GLuint getGlName() const
	{
		ANKI_ASSERT(isCreated());
		return m_glName;
	}

	/// GL object is created.
	Bool isCreated() const
	{
		return m_glName != 0;
	}

	State setStateAtomically(State newVal)
	{
		return State(m_state.exchange(I32(newVal)));
	}

	/// Check if the object has been created and if not serialize the thread.
	ANKI_USE_RESULT Error serializeRenderingThread(GrManager& manager);

	/// Should be called from GL objects for deferred deletion.
	void destroyDeferred(GrManager& manager, GlDeleteFunction deleteCallback);

protected:
	GLuint m_glName = 0; ///< OpenGL name
	mutable Atomic<I32> m_state;
};
/// @}

} // end namespace anki
