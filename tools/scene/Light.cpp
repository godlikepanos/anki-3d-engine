// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Common.h"
#include <cassert>

//==============================================================================
void exportLight(
	const Exporter& exporter,
	const aiLight& light, 
	std::ofstream& file)
{
	if(light.mType != aiLightSource_POINT || light.mType != aiLightSource_SPOT)
	{
		LOGW("Skipping light %s. Unsupported type\n", light.mName.C_Str());
		return;
	}

	file << "node = scene:new" 
		<< ((light.mType != aiLightSource_POINT) ? "Point" : "Spot") 
		<< "(\"" << light.mName.C_Str() << "\")\n";
	
	file << "lcomp = node:getLightComponent()\n";

	file << "lcomp:setDiffuseColor(" 
		<< light.mColorDiffuse[0] << ", " 
		<< light.mColorDiffuse[1] << ", " 
		<< light.mColorDiffuse[2] << ", " 
		<< light.mColorDiffuse[3]
		<< ")\n";

	file << "lcomp:setSpecularColor(" 
		<< light.mColorSpecular[0] << ", " 
		<< light.mColorSpecular[1] << ", " 
		<< light.mColorSpecular[2] << ", " 
		<< light.mColorSpecular[3]
		<< ")\n";

	aiMatrix4x4 trf;
	aiMatrix4x4::Translation(light.mPosition, trf);
	writeNodeTransform(exporter, file, "node", trf);

	switch(light.mType)
	{
	case aiLightSource_POINT:
		{
			file << "\t\t<type>point</type>\n";

			// At this point I want the radius and have the attenuation factors
			// att = Ac + Al*d + Aq*d^2. When d = r then att = 0.0. Also if we 
			// assume that Ac is 0 then:
			// 0 = Al*r + Aq*r^2. Solving by r is easy
			float r = -light.mAttenuationLinear / light.mAttenuationQuadratic;
			file << "\t\t<radius>" << r << "</radius>\n";
			break;
		}
	case aiLightSource_SPOT:
		file << "\t\t<type>spot</type>\n";
		break;
	default:
		assert(0);
		break;
	}
}
