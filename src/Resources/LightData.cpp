#include <cstring>
#include <GL/glew.h>
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
	radius(1.0),
	castsShadow_(false),
	distance(3.0),
	fovX(M::PI/4.0),
	fovY(M::PI/4.0)
{}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
bool LightData::load(const char* filename)
{
	Scanner scanner;
	if(!scanner.loadFile(filename))
	{
		return false;
	}

	const Scanner::Token* token;

	while(true)
	{
		token = &scanner.getNextToken();

		// DIFFUSE_COL
		if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "DIFFUSE_COLOR"))
		{
			Parser::parseArrOfNumbers<float>(scanner, true, true, 3, &diffuseCol[0]);
		}
		// SPECULAR_COL
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "SPECULAR_COLOR"))
		{
			Parser::parseArrOfNumbers<float>(scanner, true, true, 3, &specularCol[0]);
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
			texture->setTexParameter(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			texture->setTexParameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
	}
	
	return true;
}
