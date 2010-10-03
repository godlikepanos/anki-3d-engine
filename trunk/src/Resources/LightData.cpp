#include <cstring>
#include "LightData.h"
#include "Parser.h"
#include "Texture.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
LightData::LightData():
	Resource(RT_LIGHT_PROPS),
	diffuseCol(0.5),
	specularCol(0.5),
	castsShadow_(false),
	radius(1.0),
	distance(3.0),
	fovX(M::PI/4.0),
	fovY(M::PI/4.0)
{}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool LightData::load(const char* filename)
{
	try
	{
		Scanner scanner(filename);
		const Scanner::Token* token;

		// First of all get the type
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_IDENTIFIER || strcmp(token->getValue().getString(), "type"))
		{
			PARSE_ERR_EXPECTED("type");
			return false;
		}

		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_IDENTIFIER)
		{
			if(!strcmp(token->getValue().getString(), "LT_SPOT"))
			{
				type = LT_SPOT;
			}
			else if(!strcmp(token->getValue().getString(), "LT_POINT"))
			{
				type = LT_POINT;
			}
			else
			{
				PARSE_ERR_EXPECTED("LT_SPOT or LT_POINT");
				return false;
			}
		}


		while(true)
		{
			token = &scanner.getNextToken();

			// diffuseCol
			if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "diffuseCol"))
			{
				Parser::parseMathVector(scanner, diffuseCol);
			}
			// SPECULAR_COL
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "SPECULAR_COLOR"))
			{
				Parser::parseMathVector(scanner, specularCol);
			}
			// RADIUS
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "RADIUS"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSE_ERR_EXPECTED("number");
					return false;
				}

				radius = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																															 float(token->getValue().getInt());
			}
			// CASTS_SHADOW
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "CASTS_SHADOW"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER || token->getDataType() != Scanner::DT_INT)
				{
					PARSE_ERR_EXPECTED("number");
					return false;
				}

				castsShadow_ = token->getValue().getInt();
			}
			// DISTANCE
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "DISTANCE"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSE_ERR_EXPECTED("number");
					return false;
				}

				distance = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																																 float(token->getValue().getInt());
				type = LT_SPOT;
			}
			// FOV_X
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "FOV_X"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSE_ERR_EXPECTED("number");
					return false;
				}

				fovX = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																														 float(token->getValue().getInt());
				type = LT_SPOT;
			}
			// FOV_Y
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "FOV_Y"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_NUMBER)
				{
					PARSE_ERR_EXPECTED("number");
					return false;
				}

				fovY = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
																														 float(token->getValue().getInt());
				type = LT_SPOT;
			}
			// TEXTURE
			else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "TEXTURE"))
			{
				token = &scanner.getNextToken();
				if(token->getCode() != Scanner::TC_STRING)
				{
					PARSE_ERR_EXPECTED("string");
					return false;
				}

				texture.loadRsrc(token->getValue().getString());
				texture->setRepeat(false);
				texture->setAnisotropy(0);
				texture->setFiltering(Texture::TFT_LINEAR);
				type = LT_SPOT;
			}
			// end of file
			else if(token->getCode() == Scanner::TC_EOF)
			{
				break;
			}
			// other crap
			else
			{
				PARSE_ERR_UNEXPECTED();
				return false;
			}
		} // end while
	}
	catch(std::exception& e)
	{
		ERROR(e.what());
		return false;
	}
	
	// sanity checks
	if(type == LT_SPOT && texture.get() == NULL)
	{
		ERROR("Spot light should have texture");
		return false;
	}

	return true;
}
