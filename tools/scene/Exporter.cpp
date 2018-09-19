// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Exporter.h"
#include <iostream>

static const char* XML_HEADER = R"(<?xml version="1.0" encoding="UTF-8" ?>)";

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

static std::string getMeshName(const aiMesh& mesh)
{
	return std::string(mesh.mName.C_Str());
}

/// Walk the node hierarchy and find the node.
static const aiNode* findNodeWithName(const std::string& name, const aiNode* node, unsigned* depth = nullptr)
{
	if(node == nullptr || node->mName.C_Str() == name)
	{
		return node;
	}

	if(depth)
	{
		++(*depth);
	}

	const aiNode* out = nullptr;

	// Go to children
	for(unsigned i = 0; i < node->mNumChildren; i++)
	{
		out = findNodeWithName(name, node->mChildren[i]);
		if(out)
		{
			break;
		}
	}

	return out;
}

static std::vector<std::string> tokenize(const std::string& source)
{
	const char* delimiter = " ";
	bool keepEmpty = false;
	std::vector<std::string> results;

	size_t prev = 0;
	size_t next = 0;

	while((next = source.find_first_of(delimiter, prev)) != std::string::npos)
	{
		if(keepEmpty || (next - prev != 0))
		{
			results.push_back(source.substr(prev, next - prev));
		}

		prev = next + 1;
	}

	if(prev < source.size())
	{
		results.push_back(source.substr(prev));
	}

	return results;
}

template<int N, typename Arr>
static void stringToFloatArray(const std::string& in, Arr& out)
{
	std::vector<std::string> tokens = tokenize(in);
	if(tokens.size() != N)
	{
		ERROR("Failed to parse %s", in.c_str());
	}

	int count = 0;
	for(const std::string& s : tokens)
	{
		out[count] = std::stof(s);
		++count;
	}
}

static void removeScale(aiMatrix4x4& m)
{
	aiVector3D xAxis(m.a1, m.b1, m.c1);
	aiVector3D yAxis(m.a2, m.b2, m.c2);
	aiVector3D zAxis(m.a3, m.b3, m.c3);

	float scale = xAxis.Length();
	m.a1 /= scale;
	m.b1 /= scale;
	m.c1 /= scale;

	scale = yAxis.Length();
	m.a2 /= scale;
	m.b2 /= scale;
	m.c2 /= scale;

	scale = zAxis.Length();
	m.a3 /= scale;
	m.b3 /= scale;
	m.c3 /= scale;
}

static float getUniformScale(const aiMatrix4x4& m)
{
	const float SCALE_THRESHOLD = 0.01; // 1 cm

	aiVector3D xAxis(m.a1, m.b1, m.c1);
	aiVector3D yAxis(m.a2, m.b2, m.c2);
	aiVector3D zAxis(m.a3, m.b3, m.c3);

	float scale = xAxis.Length();
	if(std::abs(scale - yAxis.Length()) > SCALE_THRESHOLD || std::abs(scale - zAxis.Length()) > SCALE_THRESHOLD)
	{
		ERROR("No uniform scale in the matrix");
	}

	return scale;
}

static aiVector3D getNonUniformScale(const aiMatrix4x4& m)
{
	aiVector3D xAxis(m.a1, m.b1, m.c1);
	aiVector3D yAxis(m.a2, m.b2, m.c2);
	aiVector3D zAxis(m.a3, m.b3, m.c3);

	aiVector3D scale;
	scale[0] = xAxis.Length();
	scale[1] = yAxis.Length();
	scale[2] = zAxis.Length();

	return scale;
}

std::string Exporter::getMaterialName(const aiMaterial& mtl)
{
	aiString ainame;
	std::string name;
	if(mtl.Get(AI_MATKEY_NAME, ainame) == AI_SUCCESS)
	{
		name = ainame.C_Str();
	}
	else
	{
		ERROR("Material's name is missing");
	}

	return name;
}

aiMatrix4x4 Exporter::toAnkiMatrix(const aiMatrix4x4& in) const
{
	static const aiMatrix4x4 toLeftHanded(1, 0, 0, 0, 0, 0, 1, 0, 0, -1, 0, 0, 0, 0, 0, 1);

	static const aiMatrix4x4 toLeftHandedInv(1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1);

	if(m_flipyz)
	{
		return toLeftHanded * in * toLeftHandedInv;
	}
	else
	{
		return in;
	}
}

aiMatrix3x3 Exporter::toAnkiMatrix(const aiMatrix3x3& in) const
{
	static const aiMatrix3x3 toLeftHanded(1, 0, 0, 0, 0, 1, 0, -1, 0);

	static const aiMatrix3x3 toLeftHandedInv(1, 0, 0, 0, 0, -1, 0, 1, 0);

	if(m_flipyz)
	{
		return toLeftHanded * in;
	}
	else
	{
		return in;
	}
}

void Exporter::writeTransform(const aiMatrix4x4& inmat)
{
	aiMatrix4x4 mat = inmat;
	std::ofstream& file = m_sceneFile;

	float pos[3];
	pos[0] = mat[0][3];
	pos[1] = mat[1][3];
	pos[2] = mat[2][3];

	file << "trf = Transform.new()\n";

	file << "trf:setOrigin(Vec4.new(" << pos[0] << ", " << pos[1] << ", " << pos[2] << ", 0))\n";

	float scale = getUniformScale(mat);
	removeScale(mat);

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
				file << mat[j][i];
			}

			if(!(i == 3 && j == 2))
			{
				file << ", ";
			}
		}
	}
	file << ")\n";

	file << "trf:setRotation(rot)\n";
	file << "trf:setScale(" << scale << ")\n";
}

void Exporter::writeNodeTransform(const std::string& node, const aiMatrix4x4& mat)
{
	std::ofstream& file = m_sceneFile;

	writeTransform(mat);

	file << node << ":getSceneNodeBase():getMoveComponent():setLocalTransform(trf)\n";
}

const aiMesh& Exporter::getMeshAt(unsigned index) const
{
	assert(index < m_scene->mNumMeshes);
	return *m_scene->mMeshes[index];
}

const aiMaterial& Exporter::getMaterialAt(unsigned index) const
{
	assert(index < m_scene->mNumMaterials);
	return *m_scene->mMaterials[index];
}

std::string Exporter::getModelName(const Model& model) const
{
	std::string name = getMeshName(getMeshAt(model.m_meshIndex));

	name += getMaterialName(getMaterialAt(model.m_materialIndex));

	return name;
}

void Exporter::exportSkeleton(const aiMesh& mesh) const
{
	assert(mesh.HasBones());
	std::string name = mesh.mName.C_Str();
	std::fstream file;
	LOGI("Exporting skeleton %s", name.c_str());

	// Find the root bone
	unsigned minDepth = 0xFFFFFFFF;
	std::string rootBoneName;
	for(uint32_t i = 0; i < mesh.mNumBones; i++)
	{
		const aiBone& bone = *mesh.mBones[i];
		unsigned depth = 0;
		const aiNode* node = findNodeWithName(bone.mName.C_Str(), m_scene->mRootNode, &depth);
		if(!node)
		{
			ERROR("Bone \"%s\" was not found in the scene hierarchy", bone.mName.C_Str());
		}

		if(depth < minDepth)
		{
			minDepth = depth;
			rootBoneName = bone.mName.C_Str();
		}
	}
	assert(!rootBoneName.empty());

	// Open file
	file.open(m_outputDirectory + name + ".ankiskel", std::ios::out);

	file << XML_HEADER << "\n";
	file << "<skeleton>\n";
	file << "\t<bones>\n";

	for(uint32_t i = 0; i < mesh.mNumBones; i++)
	{
		const aiBone& bone = *mesh.mBones[i];

		file << "\t\t<bone>\n";

		// <name>
		file << "\t\t\t<name>" << bone.mName.C_Str() << "</name>\n";

		// <boneTransform>
		aiMatrix4x4 akMat = toAnkiMatrix(bone.mOffsetMatrix);
		file << "\t\t\t<boneTransform>";
		for(unsigned j = 0; j < 4; j++)
		{
			for(unsigned i = 0; i < 4; i++)
			{
				file << akMat[j][i] << " ";
			}
		}
		file << "</boneTransform>\n";

		// <transform>
		const aiNode* node = findNodeWithName(bone.mName.C_Str(), m_scene->mRootNode);
		assert(node);

		akMat = toAnkiMatrix(node->mTransformation);
		file << "\t\t\t<transform>";
		for(unsigned j = 0; j < 4; j++)
		{
			for(unsigned i = 0; i < 4; i++)
			{
				file << akMat[j][i] << " ";
			}
		}
		file << "</transform>\n";

		// <parent>
		if(bone.mName.C_Str() != rootBoneName)
		{
			file << "\t\t\t<parent>" << node->mParent->mName.C_Str() << "</parent>\n";
		}

		file << "\t\t</bone>\n";
	}

	file << "\t</bones>\n";
	file << "</skeleton>\n";
}

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
	file << "\t\t\t<mesh>" << m_rpath << getMeshName(getMeshAt(model.m_meshIndex)) << ".ankimesh</mesh>\n";

	// Write mesh1
	if(!model.m_lod1MeshName.empty())
	{
		bool found = false;
		for(unsigned i = 0; i < m_scene->mNumMeshes; i++)
		{
			if(m_scene->mMeshes[i]->mName.C_Str() == model.m_lod1MeshName)
			{
				file << "\t\t\t<mesh1>" << m_rpath << getMeshName(getMeshAt(i)) << ".ankimesh</mesh1>\n";
				found = true;
				break;
			}
		}

		if(!found)
		{
			ERROR("Couldn't find the LOD1 %s", model.m_lod1MeshName.c_str());
		}
	}

	// Write material
	const aiMaterial& mtl = *m_scene->mMaterials[model.m_materialIndex];
	if(mtl.mAnKiProperties.find("material_override") == mtl.mAnKiProperties.end())
	{
		file << "\t\t\t<material>" << m_rpath << getMaterialName(getMaterialAt(model.m_materialIndex))
			 << ".ankimtl</material>\n";
	}
	else
	{
		file << "\t\t\t<material>" << mtl.mAnKiProperties.at("material_override") << "</material>\n";
	}

	// End patches
	file << "\t\t</modelPatch>\n";
	file << "\t</modelPatches>\n";

	// Skeleton
	const aiMesh& aimesh = *m_scene->mMeshes[model.m_meshIndex];
	if(aimesh.HasBones())
	{
		exportSkeleton(aimesh);
		file << "\t<skeleton>" << m_rpath << aimesh.mName.C_Str() << ".ankiskel</skeleton>\n";
	}

	file << "</model>\n";
}

void Exporter::exportLight(const aiLight& light)
{
	std::ofstream& file = m_sceneFile;

	LOGI("Exporting light %s", light.mName.C_Str());

	if(light.mType != aiLightSource_POINT && light.mType != aiLightSource_SPOT)
	{
		LOGW("Skipping light %s. Unsupported type (0x%x)", light.mName.C_Str(), light.mType);
		return;
	}

	if(light.mAttenuationLinear != 0.0)
	{
		LOGW("Skipping light %s. Linear attenuation is not 0.0", light.mName.C_Str());
		return;
	}

	file << "\nnode = scene:new" << ((light.mType == aiLightSource_POINT) ? "Point" : "Spot") << "LightNode(\""
		 << light.mName.C_Str() << "\")\n";

	file << "lcomp = node:getSceneNodeBase():getLightComponent()\n";

	// Colors
	// aiColor3D linear = computeLightColor(light.mColorDiffuse);
	aiVector3D linear(light.mColorDiffuse[0], light.mColorDiffuse[1], light.mColorDiffuse[2]);
	file << "lcomp:setDiffuseColor(Vec4.new(" << linear[0] << ", " << linear[1] << ", " << linear[2] << ", 1))\n";

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
		float r = sqrt(light.mAttenuationConstant / light.mAttenuationQuadratic);
		file << "lcomp:setRadius(" << r << ")\n";
	}
	break;
	case aiLightSource_SPOT:
	{
		float dist = sqrt(light.mAttenuationConstant / light.mAttenuationQuadratic);

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
	const aiNode* node = findNodeWithName(light.mName.C_Str(), m_scene->mRootNode);

	if(node == nullptr)
	{
		ERROR("Couldn't find node for light %s", light.mName.C_Str());
	}

	aiMatrix4x4 rot;
	aiMatrix4x4::RotationX(-3.1415 / 2.0, rot);
	writeNodeTransform("node", toAnkiMatrix(node->mTransformation * rot));

	// Extra
	if(light.mProperties.find("shadow") != light.mProperties.end())
	{
		if(light.mProperties.at("shadow") == "true")
		{
			file << "lcomp:setShadowEnabled(1)\n";
		}
		else
		{
			file << "lcomp:setShadowEnabled(0)\n";
		}
	}

	if(light.mProperties.find("lens_flare") != light.mProperties.end())
	{
		file << "node:loadLensFlare(\"" << light.mProperties.at("lens_flare") << "\")\n";
	}

	bool lfCompRetrieved = false;

	if(light.mProperties.find("lens_flare_first_sprite_size") != light.mProperties.end())
	{
		if(!lfCompRetrieved)
		{
			file << "lfcomp = node:getSceneNodeBase():getLensFlareComponent()\n";
			lfCompRetrieved = true;
		}

		aiVector3D vec;
		stringToFloatArray<2>(light.mProperties.at("lens_flare_first_sprite_size"), vec);
		file << "lfcomp:setFirstFlareSize(Vec2.new(" << vec[0] << ", " << vec[1] << "))\n";
	}

	if(light.mProperties.find("lens_flare_color") != light.mProperties.end())
	{
		if(!lfCompRetrieved)
		{
			file << "lfcomp = node:getSceneNodeBase():getLensFlareComponent()\n";
			lfCompRetrieved = true;
		}

		aiVector3D vec;
		stringToFloatArray<4>(light.mProperties.at("lens_flare_color"), vec);
		file << "lfcomp:setColorMultiplier(Vec4.new(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3]
			 << "))\n";
	}

	bool eventCreated = false;
	if(light.mProperties.find("light_event_intensity") != light.mProperties.end())
	{
		if(!eventCreated)
		{
			file << "event = events:newLightEvent(0.0, -1.0, node:getSceneNodeBase())\n";
			eventCreated = true;
		}

		aiVector3D vec;
		stringToFloatArray<4>(light.mProperties.at("light_event_intensity"), vec);

		file << "event:setIntensityMultiplier(Vec4.new(" << vec[0] << ", " << vec[1] << ", " << vec[2] << ", " << vec[3]
			 << "))\n";
	}

	if(light.mProperties.find("light_event_frequency") != light.mProperties.end())
	{
		if(!eventCreated)
		{
			file << "event = events:newLightEvent(0.0, -1.0, node:getSceneNodeBase())\n";
			eventCreated = true;
		}

		float vec[2];
		stringToFloatArray<2>(light.mProperties.at("light_event_frequency"), vec);

		file << "event:setFrequency(" << vec[0] << ", " << vec[1] << ")\n";
	}
}

void Exporter::exportAnimation(const aiAnimation& anim, unsigned index)
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
				file << "\t\t\t\t<key><time>" << key.mTime << "</time><value>" << key.mValue[0] << " " << key.mValue[2]
					 << " " << -key.mValue[1] << "</value></key>\n";
			}
			else
			{
				file << "\t\t\t\t<key><time>" << key.mTime << "</time><value>" << key.mValue[0] << " " << key.mValue[1]
					 << " " << key.mValue[2] << "</value></key>\n";
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
			// aiQuaternion quat(key.mValue);

			file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
				 << "<value>" << quat.x << " " << quat.y << " " << quat.z << " " << quat.w << "</value></key>\n";
		}
		file << "\t\t\t</rotationKeys>\n";

		// Scale
		file << "\t\t\t<scalingKeys>\n";
		for(uint32_t j = 0; j < nAnim.mNumScalingKeys; j++)
		{
			const aiVectorKey& key = nAnim.mScalingKeys[j];

			// Note: only uniform scale
			file << "\t\t\t\t<key><time>" << key.mTime << "</time>"
				 << "<value>" << ((key.mValue[0] + key.mValue[1] + key.mValue[2]) / 3.0) << "</value></key>\n";
		}
		file << "\t\t\t</scalingKeys>\n";

		file << "\t\t</channel>\n";
	}

	file << "\t</channels>\n";
	file << "</animation>\n";
}

void Exporter::exportCamera(const aiCamera& cam)
{
	std::ofstream& file = m_sceneFile;

	LOGI("Exporting camera %s", cam.mName.C_Str());

	// Write the main node
	file << "\nnode = scene:newPerspectiveCameraNode(\"" << cam.mName.C_Str() << "\")\n";

	file << "scene:setActiveCameraNode(node:getSceneNodeBase())\n";

	file << "node:setAll(" << cam.mHorizontalFOV << ", "
		 << "1.0 / getMainRenderer():getAspectRatio() * " << cam.mHorizontalFOV << ", " << cam.mClipPlaneNear << ", "
		 << cam.mClipPlaneFar << ")\n";

	// Find the node
	const aiNode* node = findNodeWithName(cam.mName.C_Str(), m_scene->mRootNode);
	if(node == nullptr)
	{
		ERROR("Couldn't find node for camera %s", cam.mName.C_Str());
	}

	aiMatrix4x4 rot;
	aiMatrix4x4::RotationX(-3.1415 / 2.0, rot);
	writeNodeTransform("node", toAnkiMatrix(node->mTransformation * rot));
}

void Exporter::load()
{
	LOGI("Loading file %s", &m_inputFilename[0]);

	const int smoothAngle = 170;

	m_importer.SetPropertyFloat(AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE, smoothAngle);

	unsigned flags = 0
					 //| aiProcess_FindInstances
					 | aiProcess_JoinIdenticalVertices
					 //| aiProcess_SortByPType
					 | aiProcess_ImproveCacheLocality | aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials
					 | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals;

	const aiScene* scene = m_importer.ReadFile(m_inputFilename, flags | aiProcess_Triangulate);

	if(!scene)
	{
		ERROR("%s", m_importer.GetErrorString());
	}

	m_scene = scene;

	// Load without triangulation
	m_importerNoTriangles.SetPropertyFloat(AI_CONFIG_PP_CT_MAX_SMOOTHING_ANGLE, smoothAngle);

	scene = m_importerNoTriangles.ReadFile(m_inputFilename, flags);

	if(!scene)
	{
		ERROR("%s", m_importerNoTriangles.GetErrorString());
	}

	m_sceneNoTriangles = scene;
}

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
		unsigned mtlIndex = m_scene->mMeshes[meshIndex]->mMaterialIndex;

		// Check properties
		std::string lod1MeshName;
		std::string collisionMesh;
		bool special = false;
		for(const auto& prop : m_scene->mMeshes[meshIndex]->mProperties)
		{
			if(prop.first == "particles")
			{
				ParticleEmitter p;
				p.m_filename = prop.second;
				p.m_transform = toAnkiMatrix(ainode->mTransformation);
				m_particleEmitters.push_back(p);
				special = true;
			}
			else if(prop.first == "collision" && prop.second == "true")
			{
				StaticCollisionNode n;
				n.m_meshIndex = meshIndex;
				n.m_transform = toAnkiMatrix(ainode->mTransformation);
				m_staticCollisionNodes.push_back(n);
				special = true;
			}
			else if(prop.first == "lod1")
			{
				lod1MeshName = prop.second;
				special = false;
			}
			else if(prop.first == "reflection_probe" && prop.second == "true")
			{
				ReflectionProbe probe;
				aiMatrix4x4 trf = toAnkiMatrix(ainode->mTransformation);
				probe.m_position = aiVector3D(trf.a4, trf.b4, trf.c4);

				aiVector3D scale(trf.a1, trf.b2, trf.c3);
				assert(scale.x > 0.0f && scale.y > 0.0f && scale.z > 0.0f);

				aiVector3D half = scale;
				probe.m_aabbMin = probe.m_position - half - probe.m_position;
				probe.m_aabbMax = probe.m_position + half - probe.m_position;

				m_reflectionProbes.push_back(probe);

				special = true;
			}
			else if(prop.first == "reflection_proxy" && prop.second == "true")
			{
				ReflectionProxy proxy;

				// Find proxy in the other scene
				proxy.m_meshIndex = 0xFFFFFFFF;
				for(unsigned i = 0; i < m_sceneNoTriangles->mNumMeshes; ++i)
				{
					if(m_sceneNoTriangles->mMeshes[i]->mName == m_scene->mMeshes[meshIndex]->mName)
					{
						// Found
						proxy.m_meshIndex = i;
						break;
					}
				}

				if(proxy.m_meshIndex == 0xFFFFFFFF)
				{
					ERROR("Reflection proxy mesh not found");
				}

				proxy.m_transform = toAnkiMatrix(ainode->mTransformation);
				m_reflectionProxies.push_back(proxy);

				special = true;
			}
			else if(prop.first == "occluder" && prop.second == "true")
			{
				OccluderNode occluder;

				occluder.m_meshIndex = meshIndex;
				occluder.m_transform = toAnkiMatrix(ainode->mTransformation);
				m_occluders.push_back(occluder);

				special = true;
			}
			else if(prop.first == "collision_mesh")
			{
				collisionMesh = prop.second;
				special = false;
			}
			else if(prop.first.find("decal_") == 0)
			{
				DecalNode decal;
				for(const auto& pr : m_scene->mMeshes[meshIndex]->mProperties)
				{
					if(pr.first == "decal_diffuse_atlas")
					{
						decal.m_diffuseTextureAtlasFilename = pr.second;
					}
					else if(pr.first == "decal_diffuse_sub_texture")
					{
						decal.m_diffuseSubTextureName = pr.second;
					}
					else if(pr.first == "decal_diffuse_factor")
					{
						decal.m_factors[0] = std::stof(pr.second);
					}
					else if(pr.first == "decal_normal_roughness_atlas")
					{
						decal.m_specularRoughnessAtlasFilename = pr.second;
					}
					else if(pr.first == "decal_normal_roughness_sub_texture")
					{
						decal.m_specularRoughnessSubTextureName = pr.second;
					}
					else if(pr.first == "decal_normal_roughness_factor")
					{
						decal.m_factors[1] = std::stof(pr.second);
					}
				}

				if(decal.m_diffuseTextureAtlasFilename.empty() || decal.m_diffuseSubTextureName.empty())
				{
					ERROR("Missing decal information");
				}

				aiMatrix4x4 trf = toAnkiMatrix(ainode->mTransformation);
				decal.m_size = getNonUniformScale(trf);
				removeScale(trf);
				decal.m_transform = trf;

				m_decals.push_back(decal);
				special = true;
				break;
			}
		}

		if(special)
		{
			continue;
		}

		// Create new model
		Model mdl;
		mdl.m_meshIndex = meshIndex;
		mdl.m_materialIndex = mtlIndex;
		mdl.m_lod1MeshName = lod1MeshName;
		m_models.push_back(mdl);

		// Create new node
		Node node;
		node.m_modelIndex = m_models.size() - 1;
		node.m_transform = toAnkiMatrix(ainode->mTransformation);
		node.m_group = ainode->mGroup.C_Str();
		node.m_collisionMesh = collisionMesh;
		m_nodes.push_back(node);
	}

	// Go to children
	for(uint32_t i = 0; i < ainode->mNumChildren; i++)
	{
		visitNode(ainode->mChildren[i]);
	}
}

void Exporter::exportCollisionMesh(uint32_t meshIdx)
{
	std::string name = getMeshName(getMeshAt(meshIdx));

	std::fstream file;
	file.open(m_outputDirectory + name + ".ankicl", std::ios::out);

	file << XML_HEADER << '\n';

	// Write collision mesh
	file << "<collisionShape>\n\t<type>staticMesh</type>\n\t<value>" << m_rpath << name
		 << ".ankimesh</value>\n</collisionShape>\n";
}

void Exporter::exportAll()
{
	LOGI("Exporting scene to %s", &m_outputDirectory[0]);

	//
	// Open scene file
	//
	m_sceneFile.open(m_outputDirectory + "scene.lua");
	std::ofstream& file = m_sceneFile;

	file << "local scene = getSceneGraph()\n"
		 << "local events = getEventManager()\n"
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
	for(auto n : m_staticCollisionNodes)
	{
		exportMesh(*m_scene->mMeshes[n.m_meshIndex], nullptr, 3);
		exportCollisionMesh(n.m_meshIndex);

		file << "\n";
		writeTransform(n.m_transform);

		std::string name = getMeshName(getMeshAt(n.m_meshIndex));
		std::string fname = m_rpath + name + ".ankicl";
		file << "node = scene:newStaticCollisionNode(\"" << name << "\", \"" << fname << "\", trf)\n";
	}

	//
	// Export particle emitters
	//
	int i = 0;
	for(const ParticleEmitter& p : m_particleEmitters)
	{
		std::string name = "particles" + std::to_string(i);
		file << "\nnode = scene:newParticleEmitterNode(\"" << name << "\", \"" << p.m_filename << "\")\n";

		writeNodeTransform("node", p.m_transform);
		++i;
	}

	//
	// Export probes
	//
	i = 0;
	for(const ReflectionProbe& probe : m_reflectionProbes)
	{
		std::string name = "reflprobe" + std::to_string(i);
		file << "\nnode = scene:newReflectionProbeNode(\"" << name << "\", Vec4.new(" << probe.m_aabbMin.x << ", "
			 << probe.m_aabbMin.y << ", " << probe.m_aabbMin.z << ", 0), Vec4.new(" << probe.m_aabbMax.x << ", "
			 << probe.m_aabbMax.y << ", " << probe.m_aabbMax.z << ", 0))\n";

		aiMatrix4x4 trf;
		aiMatrix4x4::Translation(probe.m_position, trf);

		writeNodeTransform("node", trf);
		++i;
	}

	//
	// Export proxies
	//
	i = 0;
	for(const ReflectionProxy& proxy : m_reflectionProxies)
	{
		const aiMesh& mesh = *m_sceneNoTriangles->mMeshes[proxy.m_meshIndex];
		exportMesh(mesh, nullptr, 4);

		std::string name = "reflproxy" + std::to_string(i);
		file << "\nnode = scene:newReflectionProxyNode(\"" << name << "\", \"" << m_rpath << mesh.mName.C_Str()
			 << ".ankimesh\")\n";

		writeNodeTransform("node", proxy.m_transform);
		++i;
	}

	//
	// Export occluders
	//
	i = 0;
	for(const OccluderNode& occluder : m_occluders)
	{
		const aiMesh& mesh = *m_scene->mMeshes[occluder.m_meshIndex];
		exportMesh(mesh, nullptr, 3);

		std::string name = "occluder" + std::to_string(i);
		file << "\nnode = scene:newOccluderNode(\"" << name << "\", \"" << m_rpath << mesh.mName.C_Str()
			 << ".ankimesh\")\n";

		writeNodeTransform("node", occluder.m_transform);
		++i;
	}

	//
	// Export decals
	//
	i = 0;
	for(const DecalNode& decal : m_decals)
	{
		std::string name = "decal" + std::to_string(i);
		file << "\nnode = scene:newDecalNode(\"" << name << "\")\n";

		writeNodeTransform("node", decal.m_transform);

		file << "decalc = node:getSceneNodeBase():getDecalComponent()\n";
		file << "decalc:setDiffuseDecal(\"" << decal.m_diffuseTextureAtlasFilename << "\", \""
			 << decal.m_diffuseSubTextureName << "\", " << decal.m_factors[0] << ")\n";
		file << "decalc:updateShape(" << decal.m_size.x << ", " << decal.m_size.y << ", " << decal.m_size.z << ")\n";

		if(!decal.m_specularRoughnessAtlasFilename.empty())
		{
			file << "decalc:setSpecularRoughnessDecal(\"" << decal.m_specularRoughnessAtlasFilename << "\", \""
				 << decal.m_specularRoughnessSubTextureName << "\", " << decal.m_factors[1] << ")\n";
		}

		++i;
	}

	//
	// Export nodes and models.
	//
	for(uint32_t i = 0; i < m_nodes.size(); i++)
	{
		Node& node = m_nodes[i];
		Model& model = m_models[node.m_modelIndex];

		// TODO If static bake transform
		exportMesh(*m_scene->mMeshes[model.m_meshIndex], nullptr, 3);

		exportMaterial(*m_scene->mMaterials[model.m_materialIndex]);

		exportModel(model);
		std::string modelName = getModelName(model);
		std::string nodeName = modelName + node.m_group + std::to_string(i);

		// Write the main node
		file << "\nnode = scene:newModelNode(\"" << nodeName << "\", \"" << m_rpath << modelName << ".ankimdl\")\n";
		writeNodeTransform("node", node.m_transform);

		// Write the collision node
		if(!node.m_collisionMesh.empty())
		{
			unsigned i = 0;
			if(node.m_collisionMesh == "self")
			{
				i = model.m_meshIndex;
			}
			else
			{
				for(; i < m_scene->mNumMeshes; i++)
				{
					if(m_scene->mMeshes[i]->mName.C_Str() == node.m_collisionMesh)
					{
						break;
					}
				}
			}

			const bool found = i < m_scene->mNumMeshes;
			if(found)
			{
				exportCollisionMesh(i);

				std::string fname = m_rpath + getMeshName(getMeshAt(i)) + ".ankicl";
				file << "node = scene:newStaticCollisionNode(\"" << nodeName << "_cl\", \"" << fname << "\", trf)\n";
			}
			else
			{
				ERROR("Couldn't find the collision_mesh %s", node.m_collisionMesh.c_str());
			}
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

	//
	// Cameras
	//
	for(unsigned i = 0; i < m_scene->mNumCameras; i++)
	{
		exportCamera(*m_scene->mCameras[i]);
	}

	LOGI("Done exporting scene!");
}
