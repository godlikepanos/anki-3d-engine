#ifndef ANKI_GL_FBO_H
#define ANKI_GL_FBO_H

#include "anki/gl/GlObject.h"
#include "anki/util/Array.h"
#include "anki/Math.h"
#include <initializer_list>

namespace anki {

class Texture;

/// @addtogroup OpenGL
/// @{

/// Frame buffer object. The class is actually a wrapper to avoid common 
/// mistakes
class Fbo: public GlObjectContextNonSharable
{
public:
	static const U MAX_COLOR_ATTACHMENTS = 4;

	typedef GlObjectContextNonSharable Base;

	/// Used as an argument on FBO creation
	struct Attachment
	{
		const Texture* texture;
		GLenum attachmentPoint;
		I32 layer; ///< Layer or face

		Attachment(const Texture* tex, GLenum attachmentPoint_)
			:	texture(tex),
				attachmentPoint(attachmentPoint_),
				layer(-1)
		{
			ANKI_ASSERT(texture != nullptr);
		}

		Attachment(const Texture* tex, GLenum attachmentPoint_, I32 layer_)
			:	texture(tex), 
				attachmentPoint(attachmentPoint_), 
				layer(layer_)
		{
			ANKI_ASSERT(texture != nullptr);
			ANKI_ASSERT(layer != -1);
		}
	};

	/// FBO target
	enum Target
	{
		DRAW_TARGET = 1 << 1,
		READ_TARGET = 1 << 2,
		ALL_TARGETS = DRAW_TARGET | READ_TARGET
	};

	/// @name Constructors/Destructor
	/// @{
	Fbo()
		: colorAttachmentsCount(0)
	{}

	/// Move
	Fbo(Fbo&& b)
		: Fbo()
	{
		*this = std::move(b);
	}

	~Fbo()
	{
		destroy();
	}
	/// @}

	/// @name Operators
	/// @{
	Fbo& operator=(Fbo&& b)
	{
		destroy();
		Base::operator=(std::forward<Base>(b));
		colorAttachmentsCount = b.colorAttachmentsCount;
		b.colorAttachmentsCount = 0;
		return *this;
	}
	/// @}

	/// Creates a new FBO
	void create(const std::initializer_list<Attachment>& attachments);

	/// Destroy it
	void destroy();

	/// Binds FBO
	void bind(Bool invalidate, const Target target = ALL_TARGETS) const
	{
		ANKI_ASSERT(isCreated());
		checkNonSharable();
		bindInternal(this, invalidate, target);
	}

	/// Unbind all targets. Unbinds both draw and read FBOs so the active is
	/// the default FBO
	static void bindDefault(
		Bool invalidate, const Target target = ALL_TARGETS)
	{
		bindInternal(nullptr, invalidate, target);
	}

	/// Blit another framebuffer to this one
	void blitFrom(const Fbo& source, const UVec2& srcMin, const UVec2& srcMax, 
		const UVec2& dstMin, const UVec2& dstMax, 
		GLbitfield mask, GLenum filter)
	{
		source.bind(false, READ_TARGET);
		bind(false, DRAW_TARGET);
		glBlitFramebuffer(srcMin.x(), srcMin.y(), srcMax.x(), srcMax.y(), 
			dstMin.x(), dstMin.y(), dstMax.x(), dstMax.y(), mask, filter);
	}	

private:
	static thread_local const Fbo* currentRead;
	static thread_local const Fbo* currentDraw;

	U8 colorAttachmentsCount;

	static GLuint getCurrentFboGlId()
	{
		GLint i;
		glGetIntegerv(GL_FRAMEBUFFER_BINDING, &i);
		return i;
	}

	static GLuint getCurrentDrawFboGlId()
	{
		GLint i;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &i);
		return i;
	}

	static GLuint getCurrentReadFboGlId()
	{
		GLint i;
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &i);
		return i;
	}

	/// Bind an FBO
	/// @param fbo May be nullptr
	static void bindInternal(const Fbo* fbo, Bool invalidate, 
		const Target target);

	/// Attach texture internal
	void attachTextureInternal(GLenum attachment, const Texture& tex, 
		const I32 layer);
};
/// @}

} // end namespace anki

#endif
