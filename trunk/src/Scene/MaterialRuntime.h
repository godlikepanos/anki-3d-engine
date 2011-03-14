#ifndef MATERIAL_RUNTIME_H
#define MATERIAL_RUNTIME_H

#include "MtlUserDefinedVar.h"


/// @todo
class MtlUserDefinedVarRuntime
{
	public:
		MtlUserDefinedVarRuntime(const MtlUserDefinedVar& rsrc);

		/// @name Accessors
		/// @{
		const MtlUserDefinedVar& getMtlUserDefinedVar() const {return rsrc;}
		/// @}

	private:
		struct
		{
			float float_;
			Vec2 vec2;
			Vec3 vec3;
			Vec4 vec4;
		} data;

		const MtlUserDefinedVar& rsrc;
};


/// @todo
class MaterialRuntime
{
	public:
		//MaterialRuntime

	private:

};


#endif
