#include "anki/resource/Skin.h"
#include "anki/resource/Model.h"
#include "anki/resource/Skeleton.h"
#include "anki/resource/SkelAnim.h"
#include "anki/resource/Mesh.h"
#include "anki/resource/PassLevelKey.h"
#include "anki/resource/Model.h"
#include "anki/resource/Material.h"
#include "anki/misc/Xml.h"

namespace anki {

//==============================================================================

Skin::Skin()
{}

Skin::~Skin()
{}

//==============================================================================
void Skin::load(const char* filename)
{
	try
	{
		// Load
		//
		XmlDocument doc;
		doc.loadFile(filename);

		XmlElement skinEl = doc.getChildElement("skin");

		// model
		model.load(skinEl.getChildElement("model").getText());

		// skeleton
		skeleton.load(skinEl.getChildElement("skeleton").getText());

		// Anims
		XmlElement skelAnimsEl = skinEl.getChildElementOptional("skelAnims");
		if(skelAnimsEl)
		{
			XmlElement skelAnimEl = skelAnimsEl.getChildElement("skelAnim");

			do
			{
				skelAnims.push_back(SkelAnimResourcePointer());
				skelAnims.back().load(skelAnimEl.getText());

				skelAnimEl = skelAnimEl.getNextSiblingElement("skelAnim");
			} while(skelAnimEl);
		}

		// Sanity checks
		//

		// Anims and skel bones num check
		for(const SkelAnimResourcePointer& skelAnim : skelAnims)
		{
			// Bone number problem
			if(skelAnim->getBoneAnimations().size() !=
				skeleton->getBones().size())
			{
				throw ANKI_EXCEPTION("Skeleton animation \"" 
					+ skelAnim.getResourceName() + "\" and skeleton \"" 
					+ skeleton.getResourceName() 
					+ "\" dont have equal bone count");
			}
		}

		// All meshes should have vert weights
		for(const ModelPatchBase* patch : model->getModelPatches())
		{
			for(U i = 0; i < patch->getMeshesCount(); i++)
			{
				PassLevelKey key;
				key.level = i;
				const MeshBase& meshBase = patch->getMeshBase(key);
				if(!meshBase.hasWeights())
				{
					throw ANKI_EXCEPTION("Mesh does not support HW skinning");
				}
			}
		}
	}
	catch(const std::exception& e)
	{
		throw ANKI_EXCEPTION("Skin loading failed: " + filename) << e;
	}
}

} // end namespace anki
