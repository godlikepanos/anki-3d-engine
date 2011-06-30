#ifndef SCANNER_TOKEN_H
#define SCANNER_TOKEN_H

#include <string>
#include <boost/array.hpp>
#include "ScannerCommon.h"


namespace Scanner {


/// The TokenCode is an enum that defines the Token type
enum TokenCode
{
	// general codes
	TC_ERROR, TC_EOF, TC_COMMENT, TC_NUMBER, TC_CHAR, TC_STRING, TC_IDENTIFIER,
	TC_NEWLINE,

	// keywords listed by strlen (dummy keywords at the moment)
	TC_KE,
	TC_KEY,
	TC_KEYW,
	TC_KEYWO,
	TC_KEYWOR,
	TC_KEYWORD,

	// operators (in 5s)
	TC_SCOPERESOLUTION, TC_LSQBRACKET, TC_RSQBRACKET, TC_LPAREN, TC_RPAREN,
	TC_DOT, TC_POINTERTOMEMBER, TC_LBRACKET, TC_RBRACKET, TC_COMMA,
	TC_PERIOD, TC_UPDOWNDOT, TC_QUESTIONMARK, TC_SHARP, TC_EQUAL,
	TC_NOTEQUAL, TC_LESS, TC_GREATER, TC_LESSEQUAL, TC_GREATEREQUAL,
	TC_LOGICALOR, TC_LOGICALAND, TC_PLUS, TC_MINUS, TC_STAR,
	TC_BSLASH, TC_NOT, TC_BITWISEAND, TC_BITWISEOR, TC_ONESCOMPLEMENT,
	TC_MOD, TC_XOR, TC_INC, TC_DEC, TC_SHL,
	TC_SHR, TC_ASSIGN, TC_ASSIGNADD, TC_ASSIGNSUB, TC_ASSIGNMUL,
	TC_ASSIGNDIV, TC_ASSIGNMOD, TC_ASSIGNSHL, TC_ASSIGNSHR, TC_ASSIGNAND,
	TC_ASSIGNXOR, TC_ASSIGNOR
}; // end enum TokenCode


/// The value of Token::dataType
enum TokenDataType
{
	DT_FLOAT,
	DT_INT,
	DT_CHAR,
	DT_STR
};


/// Used inside the Token, its a variant that holds the data of the Token
class TokenDataVal
{
	friend class Scanner;
	friend class Token;

	public:
		/// @name Accessors
		/// @{
		char getChar() const {return char_;} ///< Access the data as C char
		/// Access the data as unsigned int
		unsigned long getInt() const {return int_;}
		double getFloat() const {return float_;} ///< Access the data as double
		/// Access the data as C string
		const char* getString() const {return string;}
		/// @}

	private:
		/// The data as unnamed union
		union
		{
			char char_;
			unsigned long int_;
			double float_;
			/// Points to @ref Token::asString if the token is string or
			/// identifier
			char* string;
		};
}; // end class TokenDataVal


/// The Token class
class Token
{
	friend class Scanner;

	public:
		Token(): code(TC_ERROR) {}
		Token(const Token& b);
		/// Get a string with the info of the token
		std::string getInfoStr() const;
		void print() const; ///< Print info of the token

		/// @name accessors
		/// @{
		const char* getString() const {return &asString[0];}
		TokenCode getCode() const {return code;}
		TokenDataType getDataType() const {return dataType;}
		const TokenDataVal& getValue() const {return value;}
		/// @}

	private:
		boost::array<char, MAX_SCRIPT_LINE_LEN> asString;
		TokenCode code; ///< The first thing you should know about a token
		/// Additional info in case @ref code is @ref TC_NUMBER
		TokenDataType dataType;
		TokenDataVal value; ///< A value variant
};


} // end namespace


#endif
