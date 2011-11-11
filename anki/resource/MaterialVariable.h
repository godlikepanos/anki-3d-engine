#ifndef ANKI_RESOURCE_MATERIAL_VARIABLE_H
#define ANKI_RESOURCE_MATERIAL_VARIABLE_H

#include "anki/resource/MaterialCommon.h"
#include "anki/resource/ShaderProgramVariable.h"
#include "anki/util/Assert.h"
#include <GL/glew.h>
#include <string>
#include <boost/array.hpp>


namespace anki {


class ShaderProgram;


/// XXX
class MaterialVariable
{
	public:
		/// The type
		enum Type
		{
			T_USER,
			T_BUILDIN
		};
		
		/// The data union
		typedef boost::variant<float, Vec2, Vec3, Vec4, Mat3, 
			Mat4, TextureResourcePointer> Variant;

		/// Used for initialization in the constructor
		typedef std::vector<std::vector<ShaderProgram*> >
			ShaderPrograms;

		/// @name Constructors & destructor
		/// @{
		
		/// Build-in
		MaterialVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& sProgs);
			
		/// User defined
		<template Type>
		MaterialVariable(
			const char* shaderProgVarName,
			const ShaderPrograms& sProgs,
			const Type& val);
		/// @}
			
		/// @name Accessors
		/// @{
		Type getType() const
		{
			return type;
		}

		/// XXX
		const ShaderProgramVariable& getShaderProgramVariable(
			PassType p) const;

		/// Check if pass p needs this variable. Check if the shader program
		/// of p contains this variable or not
		bool inPass(uint p) const
		{
			return sProgVars[p] != NULL;
		}

		/// Get the GL data type of all the shader program variables
		GLenum getGlDataType() const
		{
			return oneSProgVar->getGlDataType();
		}

		/// Get the name of all the shader program variables
		const char* getName() const
		{
			return oneSProgVar->getName().c_str();
		}

		/// Get the type of all the shader program variables
		ShaderProgramVariable::Type getShaderProgramVariableType() const
		{
			return oneSProgVar->getType();
		}
		/// @}

	private:
		Type type;
		ShaderProgramVariables sProgVars;

		/// Keep one ShaderProgramVariable here for easy access of the common
		/// variable stuff like name or GL data type etc
		const ShaderProgramVariable* oneSProgVar;
};


inline const ShaderProgramVariable& MaterialVariable::getShaderProgramVariable(
	PassType p) const
{
	ANKI_ASSERT(inPass(p));
	return *sProgVars[p];
}


} // end namespace


#endif
