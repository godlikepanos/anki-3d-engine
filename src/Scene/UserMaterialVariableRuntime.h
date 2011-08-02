#ifndef USER_MATERIAL_VARIABLE_RUNTIME_H
#define USER_MATERIAL_VARIABLE_RUNTIME_H

#include "Util/Accessors.h"
#include "Math/Math.h"
#include <boost/variant.hpp>


class UserMaterialVariable;
class Texture;


/// XXX
class UserMaterialVariableRuntime
{
	public:
		/// The data union. The Texture resource is read-only at runtime
		typedef boost::variant<float, Vec2, Vec3, Vec4,
			const Texture*> DataVariant;

		/// Constructor
		UserMaterialVariableRuntime(const UserMaterialVariable& umv);

		/// @name Accessors
		/// @{
		GETTER_R(UserMaterialVariable, umv, getUserMaterialVariable)

		/// @}

	private:
		const UserMaterialVariable& umv; ///< Know the resource
		DataVariant data;
};


#endif
