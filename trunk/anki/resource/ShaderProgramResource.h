#ifndef ANKI_RESOURCE_SHADER_PROGRAM_RESOURCE_H
#define ANKI_RESOURCE_SHADER_PROGRAM_RESOURCE_H

#include "anki/gl/ShaderProgram.h"


namespace anki {


/// Shader program resource
class ShaderProgramResource: public ShaderProgram
{
public:
	/// Resource load
	void load(const char* filename);

	/// Used by @ref Material and @ref Renderer to create custom shaders in
	/// the cache
	/// @param sProgFPathName The file pathname of the shader prog
	/// @param preAppendedSrcCode The source code we want to write on top
	///        of the shader prog
	/// @return The file pathname of the new shader prog. Its
	///         $HOME/.anki/cache/newFNamePrefix_fName
	static std::string createSrcCodeToCache(const char* sProgFPathName,
		const char* preAppendedSrcCode);
}; 


} // end namespace


#endif
