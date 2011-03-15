#ifndef MATERIAL_RUNTIME_H
#define MATERIAL_RUNTIME_H

#include <boost/ptr_container/ptr_vector.hpp>
#include "MtlUserDefinedVarRuntime.h"


class Material;


/// @todo
class MaterialRuntime
{
	public:
		MaterialRuntime(const Material& mtl);

	private:
		const Material& mtl; ///< The resource
		boost::ptr_vector<MtlUserDefinedVarRuntime> userDefVars;
};


#endif
