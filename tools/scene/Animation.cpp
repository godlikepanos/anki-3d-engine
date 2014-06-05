// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "Common.h"

//==============================================================================
void exportAnimation(
	const Exporter& exporter,
	const aiAnimation& anim,
	uint32_t index)
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
	LOGI("Exporting animation %s\n", name.c_str());

	file.open(exporter.outDir + name + ".ankianim", std::ios::out);

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

			if(exporter.flipyz)
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

			aiMatrix3x3 mat = 
				toAnkiMatrix(key.mValue.GetMatrix(), exporter.flipyz);
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
