// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Common.h"
#include <cassert>

//==============================================================================
static const aiNode* findNodeWithName(
	const std::string& name, 
	const aiNode* node)
{
	if(node == nullptr || node->mName.C_Str() == name)
	{
		return node;
	}

	const aiNode* out = nullptr;

	// Go to children
	for(uint32_t i = 0; i < node->mNumChildren; i++)
	{
		out = findNodeWithName(name, node->mChildren[i]);
		if(out)
		{
			break;
		}
	}

	return out;
}

//==============================================================================
void exportLight(
	const Exporter& exporter,
	const aiLight& light, 
	std::ofstream& file)
{
	if(light.mType != aiLightSource_POINT && light.mType != aiLightSource_SPOT)
	{
		LOGW("Skipping light %s. Unsupported type (0x%x)\n", 
			light.mName.C_Str(), light.mType);
		return;
	}

	if(light.mAttenuationLinear != 0.0)
	{
		LOGW("Skipping light %s. Linear attenuation is not 0.0\n", 
			light.mName.C_Str());
		return;
	}

	file << "node = scene:new" 
		<< ((light.mType == aiLightSource_POINT) ? "Point" : "Spot") 
		<< "Light(\"" << light.mName.C_Str() << "\")\n";
	
	file << "lcomp = node:getLightComponent()\n";

	// Colors
	file << "lcomp:setDiffuseColor(" 
		<< light.mColorDiffuse[0] << ", " 
		<< light.mColorDiffuse[1] << ", " 
		<< light.mColorDiffuse[2] << ", " 
		<< "1.0)\n";

	file << "lcomp:setSpecularColor(" 
		<< light.mColorSpecular[0] << ", " 
		<< light.mColorSpecular[1] << ", " 
		<< light.mColorSpecular[2] << ", " 
		<< "1.0)\n";

	// Geometry
	aiVector3D direction(0.0, 0.0, 1.0);

	switch(light.mType)
	{
	case aiLightSource_POINT:
		{
			// At this point I want the radius and have the attenuation factors
			// att = Ac + Al*d + Aq*d^2. When d = r then att = 0.0. Also if we 
			// assume that Al is 0 then:
			// 0 = Ac + Aq*r^2. Solving by r is easy
			float r = 
				sqrt(light.mAttenuationConstant / light.mAttenuationQuadratic);
			file << "lcomp:setRadius(" << r << ")\n";
		}
		break;
	case aiLightSource_SPOT:
		{
			float dist = 
				sqrt(light.mAttenuationConstant / light.mAttenuationQuadratic);

			file << "lcomp:setInnerAngle(" << light.mAngleInnerCone << ")\n"
				<< "lcomp:setOuterAngle(" << light.mAngleOuterCone << ")\n"
				<< "lcomp:setDistance(" << dist << ")\n";

			direction = light.mDirection;
			break;
		}
	default:
		assert(0);
		break;
	}

	// Transform
	const aiNode* node = 
		findNodeWithName(light.mName.C_Str(), exporter.scene->mRootNode);
	
	if(node == nullptr)
	{
		ERROR("Couldn't find node for light %s", light.mName.C_Str());
	}

	writeNodeTransform(exporter, file, "node", node->mTransformation);
}
