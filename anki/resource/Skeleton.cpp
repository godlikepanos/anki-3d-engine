#include <cstring>
#include <fstream>
#include "anki/resource/Skeleton.h"
#include "anki/util/BinaryStream.h"


namespace anki {


//==============================================================================
// load                                                                        =
//==============================================================================
void Skeleton::load(const char* filename)
{
	std::ifstream fs;
	fs.open(filename);
	if(!fs.is_open())
	{
		throw ANKI_EXCEPTION("Cannot open \"" + filename + "\"");
	}

	try
	{
		BinaryStream bs(fs.rdbuf());

		// Magic word
		char magic[8];
		bs.read(magic, 8);
		if(bs.fail() || memcmp(magic, "ANKISKEL", 8))
		{
			throw ANKI_EXCEPTION("Incorrect magic word");
		}

		// Bones num
		uint bonesNum = bs.readUint();
		bones.resize(bonesNum);

		// For all bones
		for(uint i=0; i<bones.size(); i++)
		{
			Bone& bone = bones[i];
			bone.id = i;

			bone.name = bs.readString();

			for(uint j=0; j<3; j++)
			{
				bone.head[j] = bs.readFloat();
			}

			for(uint j=0; j<3; j++)
			{
				bone.tail[j] = bs.readFloat();
			}

			// Matrix
			Mat4 m4;
			for(uint j=0; j<16; j++)
			{
				m4[j] = bs.readFloat();
			}

			// Matrix for real
			bone.rotSkelSpace = m4.getRotationPart();
			bone.tslSkelSpace = m4.getTranslationPart();
			Mat4 MAi(m4.getInverse());
			bone.rotSkelSpaceInv = MAi.getRotationPart();
			bone.tslSkelSpaceInv = MAi.getTranslationPart();

			// Parent
			uint parentId = bs.readUint();
			if(parentId == 0xFFFFFFFF)
			{
				bone.parent = NULL;
			}
			else if(parentId >= bonesNum)
			{
				throw ANKI_EXCEPTION("Incorrect parent id");
			}
			else
			{
				bone.parent = &bones[parentId];
			}

			// Children
			uint childsNum = bs.readUint();

			if(childsNum > Bone::MAX_CHILDS_PER_BONE)
			{
				throw ANKI_EXCEPTION("Children for bone \"" + bone.getName() +
					"\" exceed the max");
			}

			bone.childsNum = childsNum;

			for(uint j=0; j<bone.childsNum; j++)
			{
				uint id = bs.readUint();

				if(id >= bonesNum)
				{
					throw ANKI_EXCEPTION("Incorrect child id");
				}

				bone.childs[j] = &bones[id];
			}
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION_R("Skeleton \"" + filename + "\"", e);
	}
}


} // end namespace
