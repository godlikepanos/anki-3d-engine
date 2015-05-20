// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"
#include <iostream>

//==============================================================================
// Statics                                                                     =
//==============================================================================

static const char* XML_HEADER = R"(<?xml version="1.0" encoding="UTF-8" ?>)";

//==============================================================================
static aiColor3D srgbToLinear(aiColor3D in)
{
	const float p = 1.0 / 2.4;
	aiColor3D out;
	out[0] = pow(in[0], p);
	out[1] = pow(in[1], p);
	out[2] = pow(in[2], p);
	out[3] = in[3];
	return out;
}

//==============================================================================
/// Convert from sRGB to linear and preserve energy
static aiColor3D computeLightColor(aiColor3D in)
{
	float energy = std::max(std::max(in[0], in[1]), in[2]);

	if(energy > 1.0)
	{
		in[0] /= energy;
		in[1] /= energy;
		in[2] /= energy;
	}
	else
	{
		energy = 1.0;
	}

	in = srgbToLinear(in);

	in[0] *= energy;
	in[1] *= energy;
	in[2] *= energy;

	return in;
}

//==============================================================================
/// Round up the instances count.
static uint32_t roundUpInstancesCount(uint32_t instances)
{
	if(instances == 1)
	{
		instances = 1;
	}
	else if(instances <= 4)
	{
		instances = 4;
	}
	else if(instances <= 8)
	{
		instances = 8;
	}
	else if(instances <= 16)
	{
		instances = 16;
	}
	else if(instances <= 32)
	{
		instances = 32;
	}
	else
	{
		ERROR("Too many instances %u", instances);
	}

	return instances;
}

//==============================================================================
static std::string getMaterialName(const aiMaterial& mtl, uint32_t instances)
{
	aiString ainame;
	std::string name;
	if(mtl.Get(AI_MATKEY_NAME, ainame) == AI_SUCCESS)
	{
		name = ainame.C_Str();

		if(instances > 1)
		{
			name += "_inst" + std::to_string(roundUpInstancesCount(instances));
		}
	}
	else
	{
		ERROR("Material's name is missing");
	}

	return name;
}

//==============================================================================
static std::string getMeshName(const aiMesh& mesh)
{
	return std::string(mesh.mName.C_Str());
}

//==============================================================================
/// Walk the node hierarchy and find the node.
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
// Exporter                                                                    =
//==============================================================================

//==============================================================================
aiMatrix4x4 Exporter::toAnkiMatrix(const aiMatrix4x4& in) const
{
	static const aiMatrix4x4 toLeftHanded(
		1, 0, 0, 0,
		0, 0, 1, 0,
		0, -1, 0, 0,
		0, 0, 0, 1);

	static const aiMatrix4x4 toLeftHandedInv(
		1, 0, 0, 0,
		0, 0, -1, 0,
		0, 1, 0, 0,
		0, 0, 0, 1);

	if(m_flipyz)
	{
		return toLeftHanded * in * toLeftHandedInv;
	}
	else
	{
		return in;
	}
}

//==============================================================================
aiMatrix3x3 Exporter::toAnkiMatrix(const aiMatrix3x3& in) const
{
	static const aiMatrix3x3 toLeftHanded(
		1, 0, 0,
		0, 0, 1,
		0, -1, 0);

	static const aiMatrix3x3 toLeftHandedInv(
		1, 0, 0,
		0, 0, -1,
		0, 1, 0);

	if(m_flipyz)
	{
		return toLeftHanded * in;
	}
	else
	{
		return in;
	}
}

//==============================================================================
void Exporter::writeNodeTransform(
	const std::string& node,
	const aiMatrix4x4& mat)
{
	std::ofstream& file = m_sceneFile;

	aiMatrix4x4 m = toAnkiMatrix(mat);

	float pos[3];
	pos[0] = m[0][3];
	pos[1] = m[1][3];
	pos[2] = m[2][3];

	file << node
		<< ":getSceneNodeBase():getMoveComponent():setLocalOrigin(Vec4.new("
		<< pos[0] << ", " << pos[1] << ", " << pos[2] << ", 0))\n";

	file << "rot = Mat3x4.new()\n";
	file << "rot:setAll(";
	for(unsigned j = 0; j < 3; j++)
	{
		for(unsigned i = 0; i < 4; i++)
		{
			if(i == 3)
			{
				file << "0";
			}
			else
			{
				file << m[j][i];
			}

			if(!(i == 3 && j == 2))
			{
				file << ", ";
			}
		}
	}
	file << ")\n";

	file << node
		<< ":getSceneNodeBase():getMoveComponent():setLocalRotation(rot)\n";
}

//==============================================================================
const aiMesh& Exporter::getMeshAt(unsigned index) const
{
	assert(index < m_scene->mNumMeshes);
	return *m_scene->mMeshes[index];
}

//==============================================================================
const aiMaterial& Exporter::getMaterialAt(unsigned index) const
{
	assert(index < m_scene->mNumMaterials);
	return *m_scene->mMaterials[index];
}

//==============================================================================
std::string Exporter::getModelName(const Model& model) const
{
	std::string name = getMeshName(getMeshAt(model.m_meshIndex));

	name += getMaterialName(
		getMaterialAt(model.m_materialIndex), model.m_instancesCount);

	return name;
}

//==============================================================================
void Exporter::exportSkeleton(const aiMesh& mesh) const
{
	assert(mesh.HasBones());
	std::string name = mesh.mName.C_Str();
	std::fstream file;
	LOGI("Exporting skeleton %s", name.c_str());

	// Open file
	file.open(m_outputDirectory + name + ".skel", std::ios::out);

	file << XML_HEADER << "\n";
	file << "<skeleton>\n";
	file << "\t<bones>\n";

	bool rootBoneFound = false;

	for(uint32_t i = 0; i < mesh.mNumBones; i++)
	{
		const aiBone& bone = *mesh.mBones[i];

		file << "\t\t<bone>\n";

		// <name>
		file << "\t\t\t<name>" << bone.mName.C_Str() << "</name>\n";

		if(strcmp(bone.mName.C_Str(), "root") == 0)
		{
			rootBoneFound = true;
		}

		// <transform>
		file << "\t\t\t<transform>";
		for(uint32_t j = 0; j < 16; j++)
		{
			file << bone.mOffsetMatrix[j] << " ";
		}
		file << "</transform>\n";

		file << "\t\t</bone>\n";
	}

	if(!rootBoneFound)
	{
		ERROR("There should be one bone named \"root\"");
	}

	file << "\t</bones>\n";
	file << "</skeleton>\n";
}

//==============================================================================
void Exporter::exportMaterial(
	const aiMaterial& mtl,
	uint32_t instances) const
{
	std::string diffTex;
	std::string normTex;
	std::string specColTex;
	std::string shininessTex;
	std::string dispTex;

	aiString path;

	std::string name = getMaterialName(mtl, instances);
	LOGI("Exporting material %s", name.c_str());

	// Diffuse texture
	if(mtl.GetTextureCount(aiTextureType_DIFFUSE) > 0)
	{
		if(mtl.GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
		{
			diffTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture");
		}
	}

	// Normal texture
	if(mtl.GetTextureCount(aiTextureType_NORMALS) > 0)
	{
		if(mtl.GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS)
		{
			normTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture");
		}
	}

	// Specular color
	if(mtl.GetTextureCount(aiTextureType_SPECULAR) > 0)
	{
		if(mtl.GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS)
		{
			specColTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture");
		}
	}

	// Shininess color
	if(mtl.GetTextureCount(aiTextureType_SHININESS) > 0)
	{
		if(mtl.GetTexture(aiTextureType_SHININESS, 0, &path) == AI_SUCCESS)
		{
			shininessTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture");
		}
	}

	// Height texture
	if(mtl.GetTextureCount(aiTextureType_DISPLACEMENT) > 0)
	{
		if(mtl.GetTexture(aiTextureType_DISPLACEMENT, 0, &path) == AI_SUCCESS)
		{
			dispTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture");
		}
	}

	// Write file
	static const char* diffNormSpecFragTemplate =
#include "templates/diffNormSpecFrag.h"
		;
	static const char* simpleVertTemplate =
#include "templates/simpleVert.h"
		;
	static const char* tessVertTemplate =
#include "templates/tessVert.h"
		;

	static const char* readRgbFromTextureTemplate = R"(
				<operation>
					<id>%id%</id>
					<returnType>vec3</returnType>
					<function>readRgbFromTexture</function>
					<arguments>
						<argument>%map%</argument>
						<argument>out2</argument>
					</arguments>
				</operation>)";

	static const char* readRFromTextureTemplate = R"(
				<operation>
					<id>%id%</id>
					<returnType>float</returnType>
					<function>readRFromTexture</function>
					<arguments>
						<argument>%map%</argument>
						<argument>out2</argument>
					</arguments>
				</operation>)";

	// Compose full template
	// First geometry part
	std::string materialStr;
	materialStr = R"(<?xml version="1.0" encoding="UTF-8" ?>)";
	materialStr += "\n<material>\n\t<programs>\n";
	if(dispTex.empty())
	{
		materialStr += simpleVertTemplate;
	}
	else
	{
		materialStr += tessVertTemplate;
	}

	materialStr += "\n";

	// Then fragment part
	materialStr += diffNormSpecFragTemplate;
	materialStr += "\n\t</programs>\t</material>";

	// Replace strings
	if(!dispTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%dispMap%",
			m_texrpath + dispTex);
	}

	// Diffuse
	if(!diffTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%diffuseColorInput%",
			R"(<input><type>sampler2D</type><name>uDiffuseColor</name><value>)"
			+ m_texrpath + diffTex
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%diffuseColorFunc%",
			readRgbFromTextureTemplate);

		materialStr = replaceAllString(materialStr, "%id%",
			"10");

		materialStr = replaceAllString(materialStr, "%map%",
			"uDiffuseColor");

		materialStr = replaceAllString(materialStr, "%diffuseColorArg%",
			"out10");
	}
	else
	{
		aiColor3D diffCol = {0.0, 0.0, 0.0};
		mtl.Get(AI_MATKEY_COLOR_DIFFUSE, diffCol);

		materialStr = replaceAllString(materialStr, "%diffuseColorInput%",
			R"(<input><type>vec3</type><name>uDiffuseColor</name><value>)"
			+ std::to_string(diffCol[0]) + " "
			+ std::to_string(diffCol[1]) + " "
			+ std::to_string(diffCol[2])
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%diffuseColorFunc%",
			"");

		materialStr = replaceAllString(materialStr, "%diffuseColorArg%",
			"uDiffuseColor");
	}

	// Normal
	if(!normTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%normalInput%",
			R"(<input><type>sampler2D</type><name>uNormal</name><value>)"
			+ m_texrpath + normTex
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%normalFunc%",
				R"(
				<operation>
					<id>20</id>
					<returnType>vec3</returnType>
					<function>readNormalFromTexture</function>
					<arguments>
						<argument>out0</argument>
						<argument>out1</argument>
						<argument>uNormal</argument>
						<argument>out2</argument>
					</arguments>
				</operation>)");

		materialStr = replaceAllString(materialStr, "%normalArg%",
			"out20");
	}
	else
	{
		materialStr = replaceAllString(materialStr, "%normalInput%", " ");

		materialStr = replaceAllString(materialStr, "%normalFunc%", " ");

		materialStr = replaceAllString(materialStr, "%normalArg%", "out0");
	}

	// Specular
	if(!specColTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%specularColorInput%",
			R"(<input><type>sampler2D</type><name>uSpecularColor</name><value>)"
			+ m_texrpath + specColTex
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularColorFunc%",
			readRgbFromTextureTemplate);

		materialStr = replaceAllString(materialStr, "%id%",
			"50");

		materialStr = replaceAllString(materialStr, "%map%",
			"uSpecularColor");

		materialStr = replaceAllString(materialStr, "%specularColorArg%",
			"out50");
	}
	else
	{
		aiColor3D specCol = {0.0, 0.0, 0.0};
		mtl.Get(AI_MATKEY_COLOR_SPECULAR, specCol);

		materialStr = replaceAllString(materialStr, "%specularColorInput%",
			R"(<input><type>vec3</type><name>uSpecularColor</name><value>)"
			+ std::to_string(specCol[0]) + " "
			+ std::to_string(specCol[1]) + " "
			+ std::to_string(specCol[2])
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularColorFunc%",
			"");

		materialStr = replaceAllString(materialStr, "%specularColorArg%",
			"uSpecularColor");
	}

	if(!shininessTex.empty())
	{
		materialStr = replaceAllString(materialStr, "%specularPowerInput%",
			R"(<input><type>sampler2D</type><name>uSpecularPower</name><value>)"
			+ m_texrpath + shininessTex
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularPowerValue%",
			m_texrpath + shininessTex);

		materialStr = replaceAllString(materialStr, "%specularPowerFunc%",
			readRFromTextureTemplate);

		materialStr = replaceAllString(materialStr, "%id%",
			"60");

		materialStr = replaceAllString(materialStr, "%map%",
			"uSpecularPower");

		materialStr = replaceAllString(materialStr, "%specularPowerArg%",
			"out60");
	}
	else
	{
		float shininess = 0.0;
		mtl.Get(AI_MATKEY_SHININESS, shininess);
		const float MAX_SHININESS = 511.0;
		shininess = std::min(MAX_SHININESS, shininess);
		if(shininess > MAX_SHININESS)
		{
			LOGW("Shininness exceeds %f", MAX_SHININESS);
		}

		shininess = shininess / MAX_SHININESS;

		materialStr = replaceAllString(materialStr, "%specularPowerInput%",
			R"(<input><type>float</type><name>uSpecularPower</name><value>)"
			+ std::to_string(shininess)
			+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularPowerFunc%",
			"");

		materialStr = replaceAllString(materialStr, "%specularPowerArg%",
			"uSpecularPower");
	}

	materialStr = replaceAllString(materialStr, "%maxSpecularPower%", " ");

	materialStr = replaceAllString(materialStr, "%instanced%",
		(instances > 1) ? "1" : "0");
	materialStr = replaceAllString(materialStr, "%arraySize%",
		std::to_string(roundUpInstancesCount(instances)));
	materialStr = replaceAllString(materialStr, "%diffuseMap%",
		m_texrpath + diffTex);

	// Replace texture extensions with .anki
	materialStr = replaceAllString(materialStr, ".tga", ".ankitex");
	materialStr = replaceAllString(materialStr, ".png", ".ankitex");
	materialStr = replaceAllString(materialStr, ".jpg", ".ankitex");
	materialStr = replaceAllString(materialStr, ".jpeg", ".ankitex");

	// Open and write file
	std::fstream file;
	file.open(m_outputDirectory + name + ".ankimtl", std::ios::out);
	file << materialStr;
}

//==============================================================================
void Exporter::exportModel(const Model& model) const
{
	std::string name = getModelName(model);
	LOGI("Exporting model %s", name.c_str());

	std::fstream file;
	file.open(m_outputDirectory + name + ".ankimdl", std::ios::out);

	file << XML_HEADER << '\n';
	file << "<model>\n";
	file << "\t<modelPatches>\n";

	// Start patches
	file << "\t\t<modelPatch>\n";

	// Write mesh
	file << "\t\t\t<mesh>" << m_rpath
		<< getMeshName(getMeshAt(model.m_meshIndex))
		<< ".ankimesh</mesh>\n";

	// Write material
	file << "\t\t\t<material>" << m_rpath
		<< getMaterialName(getMaterialAt(model.m_materialIndex),
			model.m_instancesCount)
		<< ".ankimtl</material>\n";

	// End patches
	file << "\t\t</modelPatch>\n";
	file << "\t</modelPatches>\n";

	file << "</model>\n";
}

//==============================================================================
void Exporter::exportLight(const aiLight& light)
{
	std::ofstream& file = m_sceneFile;

	LOGI("Exporting light %s", light.mName.C_Str());

	if(light.mType != aiLightSource_POINT && light.mType != aiLightSource_SPOT)
	{
		LOGW("Skipping light %s. Unsupported type (0x%x)",
			light.mName.C_Str(), light.mType);
		return;
	}

	if(light.mAttenuationLinear != 0.0)
	{
		LOGW("Skipping light %s. Linear attenuation is not 0.0",
			light.mName.C_Str());
		return;
	}

	file << "\nnode = scene:new"
		<< ((light.mType == aiLightSource_POINT) ? "Point" : "Spot")
		<< "Light(\"" << light.mName.C_Str() << "\")\n";

	file << "lcomp = node:getSceneNodeBase():getLightComponent()\n";

	// Colors
	aiColor3D linear = computeLightColor(light.mColorDiffuse);
	file << "lcomp:setDiffuseColor(Vec4.new("
		<< linear[0] << ", "
		<< linear[1] << ", "
		<< linear[2] << ", "
		<< "1))\n";

	linear = computeLightColor(light.mColorSpecular);
	file << "lcomp:setSpecularColor(Vec4.new("
		<< linear[0] << ", "
		<< linear[1] << ", "
		<< linear[2] << ", "
		<< "1))\n";

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

			float outer = light.mAngleOuterCone;
			float inner = light.mAngleInnerCone;
			if(outer == inner)
			{
				inner = outer / 2.0;
			}

			file << "lcomp:setInnerAngle(" << inner << ")\n"
				<< "lcomp:setOuterAngle(" << outer << ")\n"
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
		findNodeWithName(light.mName.C_Str(), m_scene->mRootNode);

	if(node == nullptr)
	{
		ERROR("Couldn't find node for light %s", light.mName.C_Str());
	}

	aiMatrix4x4 rot;
	aiMatrix4x4::RotationX(-3.1415 / 2.0, rot);
	writeNodeTransform("node", node->mTransformation * rot);

	// Extra
	if(light.mShadow)
	{
		file << "lcomp:setShadowEnabled(1)\n";
	}

	if(light.mLensFlare)
	{
		file << "node:loadLensFlare(\"" << light.mLensFlare << "\")\n";
	}

	bool lfCompRetrieved = false;

	if(light.mLensFlareFirstSpriteSize != aiVector3D(0, 0, 0))
	{
		if(!lfCompRetrieved)
		{
			file << "lfcomp = node:getSceneNodeBase():"
				<< "getLensFlareComponent()\n";
			lfCompRetrieved = true;
		}

		file << "lfcomp:setFirstFlareSize(Vec2.new("
			<< light.mLensFlareFirstSpriteSize[0] << ", "
			<< light.mLensFlareFirstSpriteSize[1] << "))\n";
	}

	if(light.mLensFlareColor != aiColor4D(0, 0, 0, 0))
	{
		if(!lfCompRetrieved)
		{
			file << "lfcomp = node:getSceneNodeBase():"
				<< "getLensFlareComponent()\n";
			lfCompRetrieved = true;
		}

		file << "lfcomp:setColorMultiplier(Vec4.new("
			<< light.mLensFlareColor.r << ", "
			<< light.mLensFlareColor.g << ", "
			<< light.mLensFlareColor.b << ", "
			<< light.mLensFlareColor.a << "))\n";
	}
}

//==============================================================================
void Exporter::exportAnimation(
	const aiAnimation& anim,
	unsigned index)
{
	// Get name
	std::string name = anim.mName.C_Str();
	if(name.size() == 0)
	{
		name = std::string("unnamed_") + std::to_string(index);
	}

	// Find if it's skeleton animation
	/*bool isSkeletalAnimation = false;
	for(uint32_t i = 0; i < scene.mNumMeshes; i++)
	{
		const aiMesh& mesh = *scene.mMeshes[i];
		if(mesh.HasBones())
		{

		}
	}*/

	std::fstream file;
	LOGI("Exporting animation %s", name.c_str());

	file.open(m_outputDirectory + name + ".ankianim", std::ios::out);

	file << XML_HEADER << "\n";
	file << "<animation>\n";

	file << "\t<channels>\n";

	for(uint32_t i = 0; i < anim.mNumChannels; i++)
	{
		const aiNodeAnim& nAnim = *anim.mChannels[i];

		file << "\t\t<channel>\n";

		// Name
		file << "\t\t\t<name>" << nAnim.mNodeName.C_Str() << "</name>\n";

		// Positions
		file << "\t\t\t<positionKeys>\n";
		for(uint32_t j = 0; j < nAnim.mNumPositionKeys; j++)
		{
			const aiVectorKey& key = nAnim.mPositionKeys[j];

			if(m_flipyz)
			{
				file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
					<< "<value>" << key.mValue[0] << " "
					<< key.mValue[2] << " " << -key.mValue[1]
					<< "</value></key>\n";
			}
			else
			{
				file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
					<< "<value>" << key.mValue[0] << " "
					<< key.mValue[1] << " " << key.mValue[2]
					<< "</value></key>\n";
			}
		}
		file << "\t\t\t</positionKeys>\n";

		// Rotations
		file << "\t\t\t<rotationKeys>\n";
		for(uint32_t j = 0; j < nAnim.mNumRotationKeys; j++)
		{
			const aiQuatKey& key = nAnim.mRotationKeys[j];

			aiMatrix3x3 mat = toAnkiMatrix(key.mValue.GetMatrix());
			aiQuaternion quat(mat);
			//aiQuaternion quat(key.mValue);

			file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
				<< "<value>" << quat.x << " " << quat.y
				<< " " << quat.z << " "
				<< quat.w << "</value></key>\n";
		}
		file << "\t\t\t</rotationKeys>\n";

		// Scale
		file << "\t\t\t<scalingKeys>\n";
		for(uint32_t j = 0; j < nAnim.mNumScalingKeys; j++)
		{
			const aiVectorKey& key = nAnim.mScalingKeys[j];

			// Note: only uniform scale
			file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
				<< "<value>"
				<< ((key.mValue[0] + key.mValue[1] + key.mValue[2]) / 3.0)
				<< "</value></key>\n";
		}
		file << "\t\t\t</scalingKeys>\n";

		file << "\t\t</channel>\n";
	}

	file << "\t</channels>\n";
	file << "</animation>\n";
}

//==============================================================================
void Exporter::load()
{
	LOGI("Loading file %s", &m_inputFilename[0]);

	//Assimp::DefaultLogger::create("", Logger::VERBOSE);

	m_importer.SetPropertyFloat(AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE, 170);

	const aiScene* scene = m_importer.ReadFile(m_inputFilename, 0
		//| aiProcess_FindInstances
		| aiProcess_Triangulate
		| aiProcess_JoinIdenticalVertices
		//| aiProcess_SortByPType
		| aiProcess_ImproveCacheLocality
		| aiProcess_OptimizeMeshes
		| aiProcess_RemoveRedundantMaterials
		| aiProcess_CalcTangentSpace
		| aiProcess_GenSmoothNormals
		);

	if(!scene)
	{
		ERROR("%s", m_importer.GetErrorString());
	}

	m_scene = scene;
}

//==============================================================================
void Exporter::visitNode(const aiNode* ainode)
{
	if(ainode == nullptr)
	{
		return;
	}

	// For every mesh of this node
	for(unsigned i = 0; i < ainode->mNumMeshes; i++)
	{
		unsigned meshIndex = ainode->mMeshes[i];
		unsigned mtlIndex =  m_scene->mMeshes[meshIndex]->mMaterialIndex;

		// Check properties
		bool special = false;
		for(const auto& prop : m_scene->mMeshes[meshIndex]->mProperties)
		{
			if(prop.first == "particles_file")
			{
				ParticleEmitter p;
				p.m_filename = prop.second;
				p.m_transform = ainode->mTransformation;
				m_particleEmitters.push_back(p);
				special = true;
			}
		}

		if(special)
		{
			continue;
		}

		// Check if it's a collsion mesh
		std::string name = m_scene->mMeshes[meshIndex]->mName.C_Str();
		if(name.find("ak_collision") == 0)
		{
			// Ignore collision meshes
			m_collisionMeshIds.push_back(meshIndex);
			continue;
		}

		if(name.find("ak_portal") == 0)
		{
			// Ignore portals
			Portal portal;
			portal.m_meshIndex = meshIndex;
			portal.m_transform = ainode->mTransformation;
			m_portals.push_back(portal);
			continue;
		}

		if(name.find("ak_sector") == 0)
		{
			// Ignore sectors
			Sector sector;
			sector.m_meshIndex = meshIndex;
			sector.m_transform = ainode->mTransformation;
			m_sectors.push_back(sector);
			continue;
		}

		// Find if there is another node with the same mesh-material-group pair
		std::vector<Node>::iterator it;
		for(it = m_nodes.begin(); it != m_nodes.end(); ++it)
		{
			const Node& node = *it;
			const Model& model = m_models[node.m_modelIndex];

			if(model.m_meshIndex == meshIndex
				&& model.m_materialIndex == mtlIndex
				&& node.m_group == ainode->mGroup.C_Str()
				&& node.m_group != "none")
			{
				break;
			}
		}

		if(it != m_nodes.end())
		{
			// A node with the same model exists. It's instanced

			Node& node = *it;
			Model& model = m_models[node.m_modelIndex];

			assert(node.m_transforms.size() > 0);
			node.m_transforms.push_back(ainode->mTransformation);

			++model.m_instancesCount;
			break;
		}

		// Create new model
		Model mdl;
		mdl.m_meshIndex = meshIndex;
		mdl.m_materialIndex = mtlIndex;
		m_models.push_back(mdl);

		// Create new node
		Node node;
		node.m_modelIndex = m_models.size() - 1;
		node.m_transforms.push_back(ainode->mTransformation);
		node.m_group = ainode->mGroup.C_Str();
		m_nodes.push_back(node);
	}

	// Go to children
	for(uint32_t i = 0; i < ainode->mNumChildren; i++)
	{
		visitNode(ainode->mChildren[i]);
	}
}

//==============================================================================
void Exporter::exportCollisionMesh(uint32_t meshIdx)
{
	std::string name = getMeshName(getMeshAt(meshIdx));

	std::fstream file;
	file.open(m_outputDirectory + name + ".ankicl", std::ios::out);

	file << XML_HEADER << '\n';

	// Write collision mesh
	file << "<collisionShape>\n\t<type>staticMesh</type>\n\t<value>"
		<< m_rpath << name
		<< ".ankimesh</value>\n</collisionShape>\n";
}

//==============================================================================
void Exporter::exportAll()
{
	LOGI("Exporting scene to %s", &m_outputDirectory[0]);

	//
	// Open scene file
	//
	m_sceneFile.open(m_outputDirectory + "scene.lua");
	std::ofstream& file = m_sceneFile;

	file << "local scene = getSceneGraph()\n"
		<< "local rot\n"
		<< "local node\n"
		<< "local inst\n"
		<< "local lcomp\n";

	//
	// Get all node/model data
	//
	visitNode(m_scene->mRootNode);

	//
	// Export collision meshes
	//
	for(auto idx : m_collisionMeshIds)
	{
		exportMesh(*m_scene->mMeshes[idx], nullptr);
		exportCollisionMesh(idx);

		std::string name = getMeshName(getMeshAt(idx));
		std::string fname = m_rpath + name + ".ankicl";
		file << "\nnode = scene:newStaticCollisionNode(\""
			<< name << "\", \"" << fname << "\")\n";
	}

	//
	// Export portals
	//
	unsigned i = 0;
	for(const Portal& portal : m_portals)
	{
		uint32_t meshIndex = portal.m_meshIndex;
		exportMesh(*m_scene->mMeshes[meshIndex], nullptr);

		std::string name = getMeshName(getMeshAt(meshIndex));
		std::string fname = m_rpath + name + ".ankimesh";
		file << "\nnode = scene:newPortal(\""
			<< name << i << "\", \"" << fname << "\")\n";

		writeNodeTransform("node", portal.m_transform);
		++i;
	}

	//
	// Export sectors
	//
	i = 0;
	for(const Sector& sector : m_sectors)
	{
		uint32_t meshIndex = sector.m_meshIndex;
		exportMesh(*m_scene->mMeshes[meshIndex], nullptr);

		std::string name = getMeshName(getMeshAt(meshIndex));
		std::string fname = m_rpath + name + ".ankimesh";
		file << "\nnode = scene:newSector(\""
			<< name << i << "\", \"" << fname << "\")\n";

		writeNodeTransform("node", sector.m_transform);
		++i;
	}

	//
	// Export particle emitters
	//
	i = 0;
	for(const ParticleEmitter& p : m_particleEmitters)
	{
		std::string name = "particles" + std::to_string(i);
		file << "\nnode = scene:newParticleEmitter(\"" << name << "\", \""
			<< p.m_filename << "\")\n";

		writeNodeTransform("node", p.m_transform);
		++i;
	}

	//
	// Export nodes and models.
	//
	for(uint32_t i = 0; i < m_nodes.size(); i++)
	{
		Node& node = m_nodes[i];
		Model& model = m_models[node.m_modelIndex];

		// TODO If not instanced bake transform
		exportMesh(*m_scene->mMeshes[model.m_meshIndex], nullptr);

		exportMaterial(*m_scene->mMaterials[model.m_materialIndex],
			model.m_instancesCount);

		exportModel(model);
		std::string modelName = getModelName(model);
		std::string nodeName = modelName + node.m_group + std::to_string(i);

		// Write the main node
		file << "\nnode = scene:newModelNode(\""
			<< nodeName << "\", \""
			<< m_rpath << modelName << ".ankimdl" << "\")\n";
		writeNodeTransform("node", node.m_transforms[0]);

		// Write instance nodes
		for(unsigned j = 1; j < node.m_transforms.size(); j++)
		{
			file << "inst = scene:newInstanceNode(\""
				<< nodeName << "_inst" << (j - 1) << "\")\n"
				<< "node:getSceneNodeBase():addChild("
				<< "inst:getSceneNodeBase())\n";

			writeNodeTransform("inst", node.m_transforms[j]);
		}
	}

	//
	// Lights
	//
	for(unsigned i = 0; i < m_scene->mNumLights; i++)
	{
		exportLight(*m_scene->mLights[i]);
	}

	//
	// Animations
	//
	for(unsigned i = 0; i < m_scene->mNumAnimations; i++)
	{
		exportAnimation(*m_scene->mAnimations[i], i);
	}

	LOGI("Done exporting scene!");
}
