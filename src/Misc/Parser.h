#ifndef PARSER_H
#define PARSER_H

#include <boost/lexical_cast.hpp>
#include "Util/Exception.h"
#include "Util/Scanner/Scanner.h"


/// It contains some functions and macros that are used pretty often while
/// parsing
namespace Parser {

/// Parser macros
#define PARSER_EXCEPTION(x) \
	EXCEPTION("Parser exception (" + scanner.getScriptName() + ':' + \
	boost::lexical_cast<std::string>(scanner.getLineNumber()) + "): " + x)

#define PARSER_EXCEPTION_EXPECTED(x) \
	PARSER_EXCEPTION("Expected " + x + " and not " + \
		scanner.getCrntToken().getInfoStr())

#define PARSER_EXCEPTION_UNEXPECTED() \
	PARSER_EXCEPTION("Unexpected token " + scanner.getCrntToken().getInfoStr())


/// This template func is used for a common operation of parsing arrays of
/// numbers
///
/// It parses expressions like this one: { 10 -0.2 123.e-10 -0x0FF } and stores
/// the result in the arr array. The acceptable types (typename Type) are
/// integer or floating point types
///
/// @param scanner The scanner that we will use
/// @param bracket If true the array starts and ends with brackets eg {10 0 -1}
/// @param signs If true the array has numbers that may contain sign
/// @param size The count of numbers of the array wa want to parse
/// @param arr The array that the func returns the numbers
/// @exception Exception
template <typename Type>
void parseArrOfNumbers(Scanner::Scanner& scanner, bool bracket, bool signs,
	uint size, Type* arr);

/// Parse a single number
/// @param scanner The scanner that we will use
/// @param sign If true expect sign or not
/// @param out The output number
template <typename Type>
void parseNumber(Scanner::Scanner& scanner, bool sign, Type& out);

/// Parses a math structure (Vec3, Vec4, Mat3 etc) with leading and following
/// brackets. Eg {0.1 0.2 0.3}
template <typename Type>
void parseMathVector(Scanner::Scanner& scanner, Type& out);

/// Parse true or false identifiers
extern bool parseBool(Scanner::Scanner& scanner);

/// Parse identifier
extern std::string parseIdentifier(Scanner::Scanner& scanner,
	const char* expectedIdentifier = NULL);

/// Is identifier
extern bool isIdentifier(const Scanner::Token* token, const char* str);

/// Parse string
extern std::string parseString(Scanner::Scanner& scanner);


} // end namespace Parser


#include "Parser.inl.h"


#endif
