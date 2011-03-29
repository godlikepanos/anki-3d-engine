#include <cstring>
#include "LightRsrc.h"
#include "Parser.h"
#include "Texture.h"


//======================================================================================================================
// Constructor                                                                                                         =
//======================================================================================================================
LightRsrc::LightRsrc()
{
	diffuseCol = Vec3(0.5);
	specularCol = Vec3(0.5);
	castsShadowFlag = false;
	radius = 1.0;
	distance = 3.0;
	fovX = M::PI / 4.0;
	fovY = M::PI / 4.0;
}


//======================================================================================================================
// load                                                                                                                =
//======================================================================================================================
void LightRsrc::load(const char* filename)
{
	Scanner scanner(filename);
	const Scanner::Token* token;
	type = LT_NUM;

	while(true)
	{
		token = &scanner.getNextToken();

		// type
		if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "type"))
		{
			token = &scanner.getNextToken();
			if(token->getCode() == Scanner::TC_IDENTIFIER)
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
					throw PARSER_EXCEPTION_EXPECTED("LT_SPOT or LT_POINT");
				}
			}
		}
		// diffuseCol
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "diffuseCol"))
		{
			Parser::parseMathVector(scanner, diffuseCol);
		}
		// specularCol
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "specularCol"))
		{
			Parser::parseMathVector(scanner, specularCol);
		}
		// radius
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "radius"))
		{
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_NUMBER)
			{
				throw PARSER_EXCEPTION_EXPECTED("number");
			}

			radius = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
			                                                     float(token->getValue().getInt());
		}
		// castsShadowFlag
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "castsShadow"))
		{
			token = &scanner.getNextToken();
			if(token->getCode() == Scanner::TC_IDENTIFIER)
			{
				if(!strcmp(token->getValue().getString(), "true"))
				{
					castsShadowFlag = true;
				}
				else if(!strcmp(token->getValue().getString(), "false"))
				{
					castsShadowFlag = false;
				}
				else
				{
					throw PARSER_EXCEPTION_EXPECTED("true or false");
				}
			}
			else
			{
				throw PARSER_EXCEPTION_EXPECTED("true or false");
			}
		}
		// distance
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "distance"))
		{
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_NUMBER)
			{
				throw PARSER_EXCEPTION_EXPECTED("number");
			}

			distance = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
			                                                       float(token->getValue().getInt());
			type = LT_SPOT;
		}
		// fovX
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "fovX"))
		{
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_NUMBER)
			{
				throw PARSER_EXCEPTION_EXPECTED("number");
			}

			fovX = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
			                                                   float(token->getValue().getInt());
			type = LT_SPOT;
		}
		// fovY
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "fovY"))
		{
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_NUMBER)
			{
				throw PARSER_EXCEPTION_EXPECTED("number");
			}

			fovY = (token->getDataType() == Scanner::DT_FLOAT) ? token->getValue().getFloat() :
			                                                   float(token->getValue().getInt());
			type = LT_SPOT;
		}
		// texture
		else if(token->getCode() == Scanner::TC_IDENTIFIER && !strcmp(token->getValue().getString(), "texture"))
		{
			token = &scanner.getNextToken();
			if(token->getCode() != Scanner::TC_STRING)
			{
				throw PARSER_EXCEPTION_EXPECTED("string");
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
			throw PARSER_EXCEPTION_UNEXPECTED();
		}
	} // end while

	
	// sanity checks
	if(type == LT_NUM)
	{
		throw EXCEPTION("File \"" + filename + "\": Forgot to set type");
	}
}
