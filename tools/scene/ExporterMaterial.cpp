// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"
#include <iostream>

void Exporter::exportMaterial(const aiMaterial& mtl) const
{
	std::string diffTex;
	std::string normTex;
	std::string specColTex;
	std::string shininessTex;
	std::string dispTex;
	std::string emissiveTex;
	std::string metallicTex;

	aiString path;

	std::string name = getMaterialName(mtl);
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

	// Emissive texture
	if(mtl.GetTextureCount(aiTextureType_EMISSIVE) > 0)
	{
		if(mtl.GetTexture(aiTextureType_EMISSIVE, 0, &path) == AI_SUCCESS)
		{
			emissiveTex = getFilename(path.C_Str());
		}
		else
		{
			ERROR("Failed to retrieve texture");
		}
	}

	// Metallic texture
	if(mtl.GetTextureCount(aiTextureType_REFLECTION) > 0)
	{
		if(mtl.GetTexture(aiTextureType_REFLECTION, 0, &path) == AI_SUCCESS)
		{
			metallicTex = getFilename(path.C_Str());
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
	if(/*dispTex.empty()*/ 1)
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
		materialStr = replaceAllString(materialStr, "%dispMap%", m_texrpath + dispTex);
	}

	// Diffuse
	if(!diffTex.empty())
	{
		materialStr = replaceAllString(materialStr,
			"%diffuseColorInput%",
			R"(<input><type>sampler2D</type><name>uDiffuseColor</name><value>)" + m_texrpath + diffTex
				+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%diffuseColorFunc%", readRgbFromTextureTemplate);

		materialStr = replaceAllString(materialStr, "%id%", "10");

		materialStr = replaceAllString(materialStr, "%map%", "uDiffuseColor");

		materialStr = replaceAllString(materialStr, "%diffuseColorArg%", "out10");
	}
	else
	{
		aiColor3D diffCol = {0.0, 0.0, 0.0};
		mtl.Get(AI_MATKEY_COLOR_DIFFUSE, diffCol);

		materialStr = replaceAllString(materialStr,
			"%diffuseColorInput%",
			R"(<input><type>vec3</type><name>uDiffuseColor</name><value>)" + std::to_string(diffCol[0]) + " "
				+ std::to_string(diffCol[1])
				+ " "
				+ std::to_string(diffCol[2])
				+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%diffuseColorFunc%", "");

		materialStr = replaceAllString(materialStr, "%diffuseColorArg%", "uDiffuseColor");
	}

	// Normal
	if(!normTex.empty())
	{
		materialStr = replaceAllString(materialStr,
			"%normalInput%",
			R"(<input><type>sampler2D</type><name>uNormal</name><value>)" + m_texrpath + normTex
				+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr,
			"%normalFunc%",
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

		materialStr = replaceAllString(materialStr, "%normalArg%", "out20");
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
		materialStr = replaceAllString(materialStr,
			"%specularColorInput%",
			R"(<input><type>sampler2D</type><name>uSpecularColor</name><value>)" + m_texrpath + specColTex
				+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularColorFunc%", readRgbFromTextureTemplate);

		materialStr = replaceAllString(materialStr, "%id%", "50");

		materialStr = replaceAllString(materialStr, "%map%", "uSpecularColor");

		materialStr = replaceAllString(materialStr, "%specularColorArg%", "out50");
	}
	else
	{
		aiColor3D specCol = {0.0, 0.0, 0.0};
		mtl.Get(AI_MATKEY_COLOR_SPECULAR, specCol);

		materialStr = replaceAllString(materialStr,
			"%specularColorInput%",
			R"(<input><type>vec3</type><name>uSpecularColor</name><value>)" + std::to_string(specCol[0]) + " "
				+ std::to_string(specCol[1])
				+ " "
				+ std::to_string(specCol[2])
				+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularColorFunc%", "");

		materialStr = replaceAllString(materialStr, "%specularColorArg%", "uSpecularColor");
	}

	// Roughness
	if(!shininessTex.empty())
	{
		materialStr = replaceAllString(materialStr,
			"%specularPowerInput%",
			R"(<input><type>sampler2D</type><name>roughness</name><value>)" + m_texrpath + shininessTex
				+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularPowerValue%", m_texrpath + shininessTex);

		materialStr = replaceAllString(materialStr, "%specularPowerFunc%", readRFromTextureTemplate);

		materialStr = replaceAllString(materialStr, "%id%", "60");

		materialStr = replaceAllString(materialStr, "%map%", "roughness");

		materialStr = replaceAllString(materialStr, "%specularPowerArg%", "out60");
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

		materialStr = replaceAllString(materialStr,
			"%specularPowerInput%",
			R"(<input><type>float</type><name>roughness</name><const>1</const><value>)" + std::to_string(shininess)
				+ R"(</value></input>)");

		materialStr = replaceAllString(materialStr, "%specularPowerFunc%", "");

		materialStr = replaceAllString(materialStr, "%specularPowerArg%", "roughness");
	}

	materialStr = replaceAllString(materialStr, "%maxSpecularPower%", " ");

	// Emission
	aiColor3D emissionCol = {0.0, 0.0, 0.0};
	mtl.Get(AI_MATKEY_COLOR_EMISSIVE, emissionCol);
	float emission = (emissionCol[0] + emissionCol[1] + emissionCol[2]) / 3.0;

	if(!emissiveTex.empty())
	{
		materialStr = replaceAllString(materialStr,
			"%emissionInput%",
			"<input><type>sampler2D</type><name>emissionTex</name><value>" + m_texrpath + emissiveTex
				+ "</value></input>)\n"
				+ "\t\t\t\t<input><type>float</type><name>emission</"
				  "name><value>"
				+ std::to_string(10.0)
				+ "</value><const>1</const></input>");

		std::string func = readRFromTextureTemplate;
		func = replaceAllString(func, "%id%", "71");
		func = replaceAllString(func, "%map%", "emissionTex");
		func += R"(
				<operation>
					<id>70</id>
					<returnType>float</returnType>
					<function>mul</function>
					<arguments>
						<argument>out71</argument>
						<argument>emission</argument>
					</arguments>
				</operation>)";

		materialStr = replaceAllString(materialStr, "%emissionFunc%", func);

		materialStr = replaceAllString(materialStr, "%map%", "emissionTex");

		materialStr = replaceAllString(materialStr, "%emissionArg%", "out70");
	}
	else
	{
		materialStr = replaceAllString(materialStr,
			"%emissionInput%",
			R"(<input><type>float</type><name>emission</name><value>)" + std::to_string(emission)
				+ R"(</value><const>1</const></input>)");

		materialStr = replaceAllString(materialStr, "%emissionFunc%", "");

		materialStr = replaceAllString(materialStr, "%emissionArg%", "emission");
	}

	// Metallic
	if(!metallicTex.empty())
	{
		materialStr = replaceAllString(materialStr,
			"%metallicInput%",
			"<input><type>sampler2D</type><name>metallicTex</name><value>" + m_texrpath + metallicTex
				+ "</value></input>");

		std::string func = readRFromTextureTemplate;
		func = replaceAllString(func, "%id%", "80");
		func = replaceAllString(func, "%map%", "metallicTex");

		materialStr = replaceAllString(materialStr, "%metallicFunc%", func);

		materialStr = replaceAllString(materialStr, "%map%", "metallicTex");

		materialStr = replaceAllString(materialStr, "%metallicArg%", "out80");
	}
	else
	{
		float metallic = 0.0;
		if(mtl.mAnKiProperties.find("metallic") != mtl.mAnKiProperties.end())
		{
			metallic = std::stof(mtl.mAnKiProperties.at("metallic"));
		}

		materialStr = replaceAllString(materialStr,
			"%metallicInput%",
			R"(<input><type>float</type><name>metallic</name><value>)" + std::to_string(metallic)
				+ R"(</value><const>1</const></input>)");

		materialStr = replaceAllString(materialStr, "%metallicFunc%", "");

		materialStr = replaceAllString(materialStr, "%metallicArg%", "metallic");
	}

	// Height to parallax
	if(!dispTex.empty())
	{
		materialStr = replaceAllString(materialStr,
			"%heightVertInput%",
			"<input><type>mat4</type><name>anki_mv"
			"</name><inShadow>0</inShadow></input>");

		materialStr = replaceAllString(materialStr,
			"%heightVertFunc%",
			R"(<operation>
					<id>2</id>
					<returnType>void</returnType>
					<function>writeParallax</function>
					<arguments>
						<argument>anki_n</argument>
						<argument>anki_mv</argument>
					</arguments>
				</operation>)");

		materialStr = replaceAllString(materialStr,
			"%heightInput%",
			"<input><type>sampler2D</type><name>heightMap</name>"
			"<value>"
				+ m_texrpath
				+ dispTex
				+ "</value></input>\n"
				  "\t\t\t\t<input><type>float</type><name>heightMapScale</name>"
				  "<value>0.05</value><const>1</const></input>");

		// At this point everyone will have to use out4 as tex coords
		materialStr = replaceAllString(materialStr, "<argument>out2</argument>", "<argument>out4</argument>");

		materialStr = replaceAllString(materialStr,
			"%heightFunc%",
			R"(<operation>
					<id>4</id>
					<returnType>vec2</returnType>
					<function>computeTextureCoordParallax</function>
					<arguments>
						<argument>heightMap</argument>
						<argument>out2</argument>
						<argument>heightMapScale</argument>
					</arguments>
				</operation>)");
	}
	else
	{
		materialStr = replaceAllString(materialStr, "%heightVertInput%", " ");
		materialStr = replaceAllString(materialStr, "%heightVertFunc%", " ");
		materialStr = replaceAllString(materialStr, "%heightInput%", " ");
		materialStr = replaceAllString(materialStr, "%heightFunc%", " ");
	}

	// Continue
	materialStr = replaceAllString(materialStr, "%diffuseMap%", m_texrpath + diffTex);

	// Subsurface
	float subsurface = 0.0;
	if(mtl.mAnKiProperties.find("subsurface") != mtl.mAnKiProperties.end())
	{
		subsurface = std::stof(mtl.mAnKiProperties.at("subsurface"));
	}

	materialStr = replaceAllString(materialStr,
		"%subsurfaceInput%",
		"<input><type>float</type><name>subsurface</name>"
		"<const>1</const><value>"
			+ std::to_string(subsurface)
			+ "</value></input>");
	materialStr = replaceAllString(materialStr, "%subsurfaceArg%", "subsurface");

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
