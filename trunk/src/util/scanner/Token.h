#ifndef SCANNER_TOKEN_H
#define SCANNER_TOKEN_H

#include <string>
#include <boost/array.hpp>
#include "Common.h"


namespace scanner {


/// The TokenCode is an enum that defines the Token type
enum TokenCode
{
	// general codes
	TC_ERROR, TC_END, TC_COMMENT, TC_NUMBER, TC_CHARACTER, TC_STRING,
	TC_IDENTIFIER, TC_NEWLINE,

	// keywords listed by strlen (dummy keywords at the moment)
	TC_KE,
	TC_KEY,
	TC_KEYW,
	TC_KEYWO,
	TC_KEYWOR,
	TC_KEYWORD,

	// operators
	TC_SCOPE_RESOLUTION, TC_L_SQ_BRACKET, TC_R_SQ_BRACKET, TC_L_PAREN, 
		TC_R_PAREN,
	TC_DOT, TC_POINTER_TO_MEMBER, TC_L_BRACKET, TC_R_BRACKET, TC_COMMA,
	TC_PERIOD, TC_UPDOWNDOT, TC_QUESTIONMARK, TC_SHARP, TC_EQUAL,
	TC_NOT_EQUAL, TC_LESS, TC_GREATER, TC_LESS_EQUAL, TC_GREATER_EQUAL,
	TC_LOGICAL_OR, TC_LOGICAL_AND, TC_PLUS, TC_MINUS, TC_STAR,
	TC_BSLASH, TC_NOT, TC_BITWISE_AND, TC_BITWISE_OR, TC_UNARAY_COMPLEMENT,
	TC_MOD, TC_XOR, TC_INC, TC_DEC, TC_SHL,
	TC_SHR, TC_ASSIGN, TC_ASSIGN_ADD, TC_ASSIGN_SUB, TC_ASSIGN_MUL,
	TC_ASSIGN_DIV, TC_ASSIGN_MOD, TC_ASSIGN_SHL, TC_ASSIGN_SHR, TC_ASSIGN_AND,
	TC_ASSIGN_XOR, TC_ASSIGN_OR, TC_BACK_SLASH
}; // end enum TokenCode


/// The value of Token::dataType
enum DataType
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
		DataType getDataType() const {return dataType;}
		const TokenDataVal& getValue() const {return value;}
		/// @}

	private:
		boost::array<char, MAX_SCRIPT_LINE_LEN> asString;
		TokenCode code; ///< The first thing you should know about a token
		/// Additional info in case @ref code is @ref TC_NUMBER
		DataType dataType;
		TokenDataVal value; ///< A value variant
};


} // end namespace


#endif
