#include "anki/resource/LightRsrc.h"
#include "anki/resource/TextureResource.h"
#include "anki/misc/PropertyTree.h"
#include "anki/core/Logger.h"
#include "anki/core/Globals.h"
#include <cstring>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>


namespace anki {


//==============================================================================
LightRsrc::LightRsrc()
{
	diffuseCol = Vec3(0.5);
	specularCol = Vec3(0.5);
	castsShadowFlag = false;
	radius = 1.0;
	distance = 3.0;
	fovX = fovY = Math::PI / 4.0;
	width = height = 1.0;
	spotLightCameraType = SLCT_PERSPECTIVE;
}


//==============================================================================
void LightRsrc::load(const char* filename)
{
	try
	{
		using namespace boost::property_tree;
		ptree rpt;
		read_xml(filename, rpt);

		const ptree& pt = rpt.get_child("light");

		//
		// type
		//
		std::string type_ = pt.get<std::string>("type");
		if(type_ == "POINT")
		{
			type = LT_POINT;
		}
		else if(type_ == "SPOT")
		{
			type = LT_SPOT;
		}
		else
		{
			throw ANKI_EXCEPTION("Incorrect type: " + type_);
		}

		//
		// diffuseCol
		//
		boost::optional<const ptree&> diffColTree =
			pt.get_child_optional("diffuseCol");
		if(diffColTree)
		{
			diffuseCol = PropertyTree::getVec3(diffColTree.get());
		}

		//
		// specularCol
		//
		boost::optional<const ptree&> specColTree =
			pt.get_child_optional("specularCol");
		if(specColTree)
		{
			specularCol = PropertyTree::getVec3(specColTree.get());
		}

		//
		// castsShadow
		//
		boost::optional<bool> castsShadow_ =
			PropertyTree::getBoolOptional(pt, "castsShadow");
		if(castsShadow_)
		{
			castsShadowFlag = castsShadow_.get();
		}

		//
		// radius
		//
		boost::optional<float> radius_ = pt.get_optional<float>("radius");
		if(radius_)
		{
			radius = radius_.get();

			if(type == LT_SPOT)
			{
				ANKI_LOGW("File \"" << filename <<
					"\": No radius for spot lights");
			}
		}

		//
		// distance
		//
		boost::optional<float> distance_ = pt.get_optional<float>("distance");
		if(distance_)
		{
			distance = distance_.get();

			if(type == LT_POINT)
			{
				ANKI_LOGW("File \"" << filename <<
					"\": No distance for point lights");
			}
		}

		//
		// fovX
		//
		boost::optional<float> fovX_ = pt.get_optional<float>("fovX");
		if(fovX_)
		{
			fovX = fovX_.get();

			if(type == LT_POINT)
			{
				ANKI_LOGW("File \"" << filename <<
					"\": No fovX for point lights");
			}
		}

		//
		// fovY
		//
		boost::optional<float> fovY_ = pt.get_optional<float>("fovY");
		if(fovY_)
		{
			fovY = fovY_.get();

			if(type == LT_POINT)
			{
				ANKI_LOGW("File \"" << filename <<
					"\": No fovY for point lights");
			}
		}

		//
		// width
		//
		boost::optional<float> width_ = pt.get_optional<float>("width");
		if(width_)
		{
			width = width_.get();

			if(type == LT_POINT)
			{
				ANKI_LOGW("File \"" << filename <<
					"\": No width for point lights");
			}
		}

		//
		// height
		//
		boost::optional<float> height_ = pt.get_optional<float>("height");
		if(height_)
		{
			height = height_.get();

			if(type == LT_POINT)
			{
				ANKI_LOGW("File \"" << filename <<
					"\": No height for point lights");
			}
		}

		//
		// texture
		//
		boost::optional<std::string> tex =
			pt.get_optional<std::string>("texture");
		if(tex)
		{
			texture.load(tex.get().c_str());

			if(type == LT_POINT)
			{
				ANKI_LOGW("File \"" << filename <<
					"\": No texture for point lights");
			}
		}

		//
		// cameraType
		//
		boost::optional<std::string> cam =
			pt.get_optional<std::string>("cameraType");
		if(cam)
		{
			if(cam.get() == "PERSPECTIVE")
			{
				spotLightCameraType = SLCT_PERSPECTIVE;
			}
			else if(cam.get() == "ORTHOGRAPHIC")
			{
				spotLightCameraType = SLCT_ORTHOGRAPHIC;
			}
			else
			{
				throw ANKI_EXCEPTION("Incorrect cameraYype: " + cam.get());
			}
		}

	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION("Material \"" + filename + "\"") << e;
	}
}


} // end namespace
