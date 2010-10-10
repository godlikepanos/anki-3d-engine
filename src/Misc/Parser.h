#ifndef PARSER_H
#define PARSER_H

#include <boost/lexical_cast.hpp>
#include "Exception.h"
#include "Scanner.h"


/// @brief It contains some functions and macros that include code commonly used in parsing
namespace Parser {


//======================================================================================================================
// Parser macros                                                                                                       =
//======================================================================================================================
#define PARSER_EXCEPTION(x) \
	EXCEPTION("Parser exception (" + scanner.getScriptName() + ':' + \
	boost::lexical_cast<std::string>(scanner.getLineNumber()) + "): " + x)

#define PARSER_EXCEPTION_EXPECTED(x) \
	PARSER_EXCEPTION("Expected " + x + " and not " + scanner.getCrntToken().getInfoStr())

#define PARSER_EXCEPTION_UNEXPECTED() \
	PARSER_EXCEPTION("Unexpected token " + scanner.getCrntToken().getInfoStr())


//======================================================================================================================
// parseArrOfNumbers                                                                                                   =
//======================================================================================================================
/**
 * This template func is used for a common operation of parsing arrays of numbers
 *
 * It parses expressions like this one: { 10 -0.2 123.e-10 -0x0FF } and stores the result in the arr array. The
 * acceptable types (typename Type) are integer or floating point types
 *
 * @param scanner The scanner that we will use
 * @param bracket If true the array starts and ends with brackets eg { 10 0 -1 }
 * @param signs If true the array has numbers that may contain sign
 * @param size The count of numbers of the array wa want to parse
 * @param arr The array that the func returns the numbers
 * @return True if the parsing was successful
*/
/*template <typename Type>
bool parseArrOfNumbers(Scanner& scanner, bool bracket, bool signs, uint size, Type* arr)
{
	const Scanner::Token* token;

	// first of all the left bracket
	if(bracket)
	{
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_LBRACKET)
		{
			PARSE_ERR_EXPECTED("{");
			return false;
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
			if(token->getCode() == Scanner::TC_MINUS)
			{
				sign = 1;
				token = &scanner.getNextToken();
			}
			else if(token->getCode() == Scanner::TC_PLUS)
			{
				sign = 0;
				token = &scanner.getNextToken();
			}

			// check if not number
			if(token->getCode() != Scanner::TC_NUMBER)
			{
				PARSE_ERR_EXPECTED("number");
				return false;
			}
		} // end if signs

		// put the number in the arr and do typecasting from int to float
		Type nmbr;

		if(token->getDataType() == Scanner::DT_FLOAT)
			nmbr = static_cast<Type>(token->getValue().getFloat());
		else
			nmbr = static_cast<Type>(token->getValue().getInt());

		arr[i] = (sign==0) ? nmbr : -nmbr;
	}

	// the last thing is the right bracket
	if(bracket)
	{
		token = &scanner.getNextToken();
		if(token->getCode() != Scanner::TC_RBRACKET)
		{
			PARSE_ERR_EXPECTED("}");
			return false;
		}
	}

	return true;
}*/


/// Parse a single number
/// @param scanner The scanner that we will use
/// @param sign If true expect sign or not
/// @param out The output number
template <typename Type>
void parseNumber(Scanner& scanner, bool sign, Type& out);


/// Parses a math structure (Vec3, Vec4, Mat3 etc) with leading and following brackets
template <typename Type>
void parseMathVector(Scanner& scanner, Type& out);


} // end namespace Parser


#include "Parser.inl.h"


#endif
