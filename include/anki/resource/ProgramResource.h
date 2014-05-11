#ifndef ANKI_RESOURCE_PROGRAM_RESOURCE_H
#define ANKI_RESOURCE_PROGRAM_RESOURCE_H

#include "anki/Gl.h"

namespace anki {

/// @addtogroup Resource
/// @{

/// Shader program resource
class ProgramResource
{
public:
	ProgramResource()
	{}
	~ProgramResource()
	{}

	const GlProgramHandle& getGlProgram() const
	{
		return m_prog;
	}

	/// Resource load
	void load(const char* filename);

	/// Load and add extra code on top of the file
	void load(const char* filename, const char* extraSrc);

	/// Used by @ref Material and @ref Renderer to create custom shaders in
	/// the cache
	/// @param filename The file pathname of the shader prog
	/// @param preAppendedSrcCode The source code we want to write on top
	///        of the shader prog
	/// @param filenamePrefix Add that at the base filename for additional 
	///        ways to identify the file in the cache
	/// @return The file pathname of the new shader prog. Its
	///         $HOME/.anki/cache/ + filenamePrefix + hash + .glsl
	static std::string createSrcCodeToCache(
		const char* filename,
		const char* preAppendedSrcCode,
		const char* filenamePrefix);

private:
	GlProgramHandle m_prog;
}; 
/// @}

} // end namespace anki

#endif
