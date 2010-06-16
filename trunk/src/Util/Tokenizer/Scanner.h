#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <sstream>
#include "Common.h"


/**
 * C++ Tokenizer
 *
 * The Scanner loads a file or an already loaded iostream and extracts the tokens. The script must be in C++ format. The
 * class does not make any kind of memory allocations so it can be fast.
 */
class Scanner
{
	//====================================================================================================================
	// Pre-nested                                                                                                        =
	//====================================================================================================================
	protected:
		static const int MAX_SCRIPT_LINE_LEN = 1024; ///< The max allowed length of a script line

	//====================================================================================================================
	// Nested                                                                                                            =
	//====================================================================================================================
	public:
		/**
		 * The TokenCode is an enum that defines the Token type
		 */
		enum TokenCode
		{
			// general codes
			TC_ERROR, TC_EOF, TC_COMMENT, TC_NUMBER, TC_CHAR, TC_STRING, TC_IDENTIFIER, TC_NEWLINE,

			// keywords listed by strlen (dummy keywords at the moment)
			TC_KE,
			TC_KEY,
			TC_KEYW,
			TC_KEYWO,
			TC_KEYWOR,
			TC_KEYWORD,

			// operators
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


		/**
		 * The value of @ref Token::dataType
		 */
		enum TokenDataType
		{
			DT_FLOAT,
			DT_INT,
			DT_CHAR,
			DT_STR
		};

		/**
		 * Used inside the @ref Token, its a variant that holds the data of the Token
		 */
		class TokenDataVal
		{
			friend class Scanner;
			friend class Token;

			private:
				/**
				 * The data as unamed union
				 */
				union
				{
					char   char_;
					ulong  int_;
					double float_;
					char*  string; ///< points to @ref Token::asString if the token is string or identifier
				};

			public:
				/**
				 * @name Accessors
				 */
				/**@{*/
				char        getChar()   const { return char_; }  ///< Access the data as C char
				ulong       getInt()    const { return int_; }   ///< Access the data as unsigned int
				float       getFloat()  const { return float_; } ///< Access the data as double
				const char* getString() const { return string; } ///< Access the data as C string
				/**@}*/
		}; // end class TokenDataVal


		/**
		 * The Token class
		 */
		struct Token
		{
			friend class Scanner;

			PROPERTY_R(TokenCode, code, getCode) ///< The first thing you should know about a token
			PROPERTY_R(TokenDataType, dataType, getDataType) ///< Additional info in case @ref code is @ref TC_NUMBER
			PROPERTY_R(TokenDataVal, value, getValue) ///< A value variant

			public:
				Token(): code(TC_ERROR) {}
				Token(const Token& b);
				const char* getString() const { return asString; }
				string getInfoStr() const; ///< Get a string with the info of the token
				void print() const; ///< Print info of the token

			private:
				char asString[1024];
		}; // end class Token


	protected:
		/**
		 * Every char in the Ascii table is binded with one characteristic code type. This helps the scanning
		 */
		enum AsciiFlag
		{
			AC_ERROR = 0,
			AC_EOF = 1,
			AC_LETTER = 2,
			AC_DIGIT = 4,
			AC_SPECIAL = 8,
			AC_WHITESPACE = 16,
			AC_QUOTE = 32,
			AC_DOUBLEQUOTE = 64
		};

		/**
		 * Reserved words like "int" "const" etc. Currently the reserved words list is being populated with dummy data
		 */
		struct ResWord
		{
			const char* string;
			TokenCode   code;
		};

	//====================================================================================================================
	// Public                                                                                                            =
	//====================================================================================================================
	public:
		/**
		 * The one and only constructor
		 * @param newlinesAsWhitespace If false the Scanner will return a newline @ref Token every time if finds a newline.
		 * True is the default behavior
		 */
		Scanner(bool newlinesAsWhitespace = true);

		~Scanner() { /* The destructor does NOTHING. The class does not make any mem allocations */ }

		bool loadFile(const char* filename); ///< Load a file to extract tokens

		/**
		 * Load a STL istream to extract tokens
		 * @param istream_ The stream from where to read
		 * @param scriptName_ The name of the stream. For error reporting
		 * @return True on success
		 */
		bool loadIstream(istream& istream_, const char* scriptName_ = "unamed-istream");

		/**
		 * Unloads the file
		 */
		void unload();

		/**
		 * Extracts all tokens and prints them. Used for debugging
		 */
		void getAllPrintAll();

		/**
		 * Get the next token from the stream
		 */
		const Token& getNextToken();

		/**
		 * Accessor for the current token
		 * @return
		 */
		const Token& getCrntToken() const { return crntToken; }

		/**
		 * Get the name of the input stream
		 */
		const char* getScriptName() const { return scriptName; }

		/**
		 * Get the current line the Scanner is processing
		 */
		int getLineNumber() const { return lineNmbr; }


	//====================================================================================================================
	// Protected                                                                                                         =
	//====================================================================================================================
	protected:
		static char eofChar; ///< Special end of file character

		static AsciiFlag asciiLookupTable []; ///< The array contains one AsciiFlag for every symbol of the ASCII table

		/**
		 * @name Reserved words
		 * Groups of ResWord grouped by the length of the ResWord::string
		 */
		/**@{*/
		static ResWord rw2 [], rw3 [], rw4 [], rw5 [], rw6 [], rw7 [];
		/**@}*/

		static ResWord* rwTable []; ///< The array contains all the groups of ResWord

		Token crntToken; ///< The current token
		char  line [MAX_SCRIPT_LINE_LEN]; ///< In contains the current line's text
		char* pchar; ///< Points somewhere to @ref line
		int   lineNmbr; ///< The number of the current line

		/**
		 * Treat newlines as whitespace. If false means that the Scanner returns (among others) newline tokens
		 */
		bool newlinesAsWhitespace;

		/**
		 * Commented lines number
		 *
		 * Used to keep track of the newlines in multiline comments so we can then return the correct number of newlines in
		 * case of newlinesAsWhitespace is false
		 */
		int commentedLines;

		/**
		 * @name Input
		 */
		/**@{*/
		ifstream inFstream; ///< The file stream. Used if the @ref Scanner is initiated using @ref loadFile
		istream* inStream; ///< Points to either @ref inFstream or an external std::istream
		char scriptName[512]; ///< The name of the input stream. Mainly used for the error messages
		/**@}*/

		/**
		 * @name Checkers
		 */
		/**@{*/
		bool checkWord();
		bool checkComment();
		bool checkNumber();
		bool checkString();
		bool checkChar();
		bool checkSpecial();
		/**@}*/

		/**
		 * It reads a new line from the iostream and it points @ref pchar to the beginning of that line
		 */
		void getLine();

		/**
		 * Get the next char from the @ref line. If @ref line empty then get new line. It returns '\\0' if we are in the
		 * end of the line
		 */
		char getNextChar();

		/**
		 * Put the char that Scanner::GetNextChar got back to the current line
		 */
		char putBackChar();

		/**
		 * Initializes the asciiLookupTable. It runs only once in the construction of the first Scanner @see Scanner()
		 */
		static void initAsciiMap();

		/**
		 * To save us from typing for example asciiLookupTable[(int)'a']
		 */
		inline AsciiFlag asciiLookup(char ch_)
		{
			return asciiLookupTable[(int)ch_];
		}
}; // end class Scanner

#endif
