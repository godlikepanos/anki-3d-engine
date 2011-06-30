#include "ScannerToken.h"
#include <cstring>
#include <cstdio>
#include <iostream>
#include <boost/lexical_cast.hpp>


namespace Scanner {


//==============================================================================
// Copy constructor                                                            =
//==============================================================================
Token::Token(const Token& b)
:	code(b.code),
	dataType(b.dataType)
{
	switch(b.dataType)
	{
		case DT_FLOAT:
			value.float_ = b.value.float_;
			break;
		case DT_INT:
			value.int_ = b.value.int_;
			break;
		case DT_CHAR:
			value.char_ = b.value.char_;
			break;
		case DT_STR:
			value.string = b.value.string;
			break;
	}
	memcpy(&asString[0], &b.asString[0], sizeof(asString));
}


//==============================================================================
// getInfoStr                                                                  =
//==============================================================================
std::string Token::getInfoStr() const
{
	char tokenInfoStr[512];
	switch(code)
	{
		case TC_COMMENT:
			return "comment";
		case TC_NEWLINE:
			return "newline";
		case TC_EOF:
			return "end of file";
		case TC_STRING:
			sprintf(tokenInfoStr, "string \"%s\"", value.string);
			break;
		case TC_CHAR:
			sprintf(tokenInfoStr, "char '%c' (\"%s\")", value.char_,
				&asString[0]);
			break;
		case TC_NUMBER:
			if(dataType == DT_FLOAT)
			{
				sprintf(tokenInfoStr, "float %f or %e (\"%s\")", value.float_,
					value.float_, &asString[0]);
			}
			else
				sprintf(tokenInfoStr, "int %lu (\"%s\")", value.int_,
					&asString[0]);
			break;
		case TC_IDENTIFIER:
			sprintf(tokenInfoStr, "identifier \"%s\"", value.string);
			break;
		case TC_ERROR:
			return "scanner error";
			break;
		default:
			if(code >= TC_KE && code <= TC_KEYWORD)
			{
				sprintf(tokenInfoStr, "reserved word \"%s\"", value.string);
			}
			else if(code>=TC_SCOPERESOLUTION && code<=TC_ASSIGNOR)
			{
				sprintf(tokenInfoStr, "operator no %d",
					code - TC_SCOPERESOLUTION);
			}
	}

	return tokenInfoStr;
}


//==============================================================================
// print                                                                       =
//==============================================================================
void Token::print() const
{
	std::cout << "Token: " << getInfoStr() << std::endl;
}


} // end namespace
