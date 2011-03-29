#ifndef MTL_USER_DEFINED_VAR_H
#define MTL_USER_DEFINED_VAR_H

#include <boost/variant.hpp>
#include "Properties.h"
#include "Math.h"
#include "SProgUniVar.h"
#include "RsrcPtr.h"


class Texture;


/// Class for user defined material variables that will be passes in to the shader
class MtlUserDefinedVar
{
	public:
		/// The renderer's FAIs
		enum Fai
		{
			MS_DEPTH_FAI, ///< Avoid it in MS
			IS_FAI, ///< Use it anywhere
			PPS_PRE_PASS_FAI, ///< Avoid it in BS
			PPS_POST_PASS_FAI ///< Use it anywhere
		};

		/// The data union
		typedef boost::variant<float, Vec2, Vec3, Vec4, RsrcPtr<Texture>, Fai> DataVariant;

		/// Contructor
		/// @tparam Type Can be std::string, Fai, float, Vec2, Vec3, Vec4
		template<typename Type>
		MtlUserDefinedVar(const SProgUniVar& sProgVar, const Type& val);

		/// @name Accessors
		/// @{
		const SProgUniVar& getUniVar() const {return sProgVar;}

		const DataVariant& getDataVariant() const {return data;}

		/// Get the value of the variant
		/// @exception boost::exception when you try to get the incorrect data type
		template<typename Type>
		const Type& get() const {return boost::get<Type>(data);}
		/// @}

	private:
		DataVariant data;
		const SProgUniVar& sProgVar; ///< Know a part of the ShaderProg resource
};


#endif
