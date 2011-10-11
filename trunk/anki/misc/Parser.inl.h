#include <cstring>
#include "anki/misc/Parser.h"
#include "anki/util/Exception.h"


namespace parser {


//==============================================================================
// parseArrOfNumbers                                                           =
//==============================================================================
template <typename Type>
void parseArrOfNumbers(scanner::Scanner& scanner, bool bracket, bool signs,
	uint size, Type* arr)
{
	const scanner::Token* token;

	// first of all the left bracket
	if(bracket)
	{
		token = &scanner.getNextToken();
		if(token->getCode() != scanner::TC_L_BRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("{");
		}
	}

	// loop to parse numbers
	for(uint i=0; i<size; i++)
	{
		token = &scanner.getNextToken();


		// check if there is a sign in front of a number
		bool sign = 0; // sign of the number. 0 for positive and 1 for negative
		if(signs)
		{
			if(token->getCode() == scanner::TC_MINUS)
			{
				sign = 1;
				token = &scanner.getNextToken();
			}
			else if(token->getCode() == scanner::TC_PLUS)
			{
				sign = 0;
				token = &scanner.getNextToken();
			}

			// check if not number
			if(token->getCode() != scanner::TC_NUMBER)
			{
				throw PARSER_EXCEPTION_EXPECTED("number");
			}
		} // end if signs

		// put the number in the arr and do typecasting from int to float
		Type nmbr;

		if(token->getDataType() == scanner::DT_FLOAT)
		{
			nmbr = static_cast<Type>(token->getValue().getFloat());
		}
		else
		{
			nmbr = static_cast<Type>(token->getValue().getInt());
		}

		arr[i] = (sign==0) ? nmbr : -nmbr;
	}

	// the last thing is the right bracket
	if(bracket)
	{
		token = &scanner.getNextToken();
		if(token->getCode() != scanner::TC_R_BRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("}");
		}
	}
}


//==============================================================================
// parseNumber                                                                 =
//==============================================================================
template <typename Type>
void parseNumber(scanner::Scanner& scanner, bool sign, Type& out)
{
	try
	{
		const scanner::Token* token = &scanner.getNextToken();
		bool negative = false;

		// check the sign if any
		if(sign)
		{
			if(token->getCode() == scanner::TC_PLUS)
			{
				negative = false;
				token = &scanner.getNextToken();
			}
			else if(token->getCode() == scanner::TC_MINUS)
			{
				negative = true;
				token = &scanner.getNextToken();
			}
			else if(token->getCode() != scanner::TC_NUMBER)
			{
				throw PARSER_EXCEPTION_EXPECTED("number or sign");
			}
		}

		// check the number
		if(token->getCode() != scanner::TC_NUMBER)
		{
			throw PARSER_EXCEPTION_EXPECTED("number");
		}

		if(token->getDataType() == scanner::DT_FLOAT)
		{
			double d = negative ? -token->getValue().getFloat() :
				token->getValue().getFloat();
			out = static_cast<Type>(d);
		}
		else
		{
			ulong l = negative ? -token->getValue().getInt() :
				token->getValue().getInt();
			out = static_cast<Type>(l);
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION(e.what());
	}
}


//==============================================================================
// parseMathVector                                                             =
//==============================================================================
template <typename Type>
void parseMathVector(scanner::Scanner& scanner, Type& out)
{
	try
	{
		const scanner::Token* token = &scanner.getNextToken();
		uint elementsNum = sizeof(Type) / sizeof(float);

		// {
		if(token->getCode() != scanner::TC_L_BRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("{");
		}

		// numbers
		for(uint i=0; i<elementsNum; i++)
		{
			double d;
			parseNumber(scanner, true, d);
			out[i] = d;
		}

		// }
		token = &scanner.getNextToken();
		if(token->getCode() != scanner::TC_R_BRACKET)
		{
			throw PARSER_EXCEPTION_EXPECTED("}");
		}
	}
	catch(std::exception& e)
	{
		throw ANKI_EXCEPTION(e.what());
	}
}


//==============================================================================
// parseBool                                                                   =
//==============================================================================
inline bool parseBool(scanner::Scanner& scanner)
{
	const char* errMsg = "identifier true or false";

	const scanner::Token* token = &scanner.getNextToken();
	if(token->getCode() != scanner::TC_IDENTIFIER)
	{
		throw PARSER_EXCEPTION_EXPECTED(errMsg);
	}

	if(!strcmp(token->getValue().getString(), "true"))
	{
		return true;
	}
	else if (!strcmp(token->getValue().getString(), "false"))
	{
		return false;
	}
	else
	{
		throw PARSER_EXCEPTION_EXPECTED(errMsg);
	}
}


//==============================================================================
// parseIdentifier                                                             =
//==============================================================================
inline std::string parseIdentifier(scanner::Scanner& scanner,
	const char* expectedIdentifier)
{
	const scanner::Token* token = &scanner.getNextToken();
	if(token->getCode() != scanner::TC_IDENTIFIER)
	{
		if(expectedIdentifier == NULL)
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier");
		}
		else
		{
			throw PARSER_EXCEPTION_EXPECTED("identifier " + expectedIdentifier);
		}
	}

	if(expectedIdentifier != NULL &&
		strcmp(token->getValue().getString(), expectedIdentifier))
	{
		throw PARSER_EXCEPTION_EXPECTED("identifier " + expectedIdentifier);
	}

	return token->getValue().getString();
}


//==============================================================================
// isIdentifier                                                                =
//==============================================================================
inline bool isIdentifier(const scanner::Token& token, const char* str)
{
	return token.getCode() == scanner::TC_IDENTIFIER &&
		!strcmp(token.getValue().getString(), str);
}


//==============================================================================
// parseString                                                                 =
//==============================================================================
inline std::string parseString(scanner::Scanner& scanner)
{
	const scanner::Token* token = &scanner.getNextToken();
	if(token->getCode() != scanner::TC_STRING)
	{
		throw PARSER_EXCEPTION_EXPECTED("string");
	}
	return token->getValue().getString();
}


} // End namespace
