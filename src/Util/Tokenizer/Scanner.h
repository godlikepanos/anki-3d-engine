#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <sstream>
#include "Common.h"


/**
 * C++ Tokenizer
 *
 * The Scanner loads a file or an already loaded iostream and extracts the tokens. The script must be in C++ format. The class does
 * not make any kind of memory allocations so it can be fast.
 */
class Scanner
{
	//===================================================================================================================================
	// private stuff                                                                                                                    =
	//===================================================================================================================================
	protected:
		static const int MAX_SCRIPT_LINE_LEN = 1024; ///< The max allowed length of a script line

		static char eofChar; ///< Special end of file character

		// checkers
		bool checkWord();
		bool checkComment();
		bool checkNumber();
		bool checkString();
		bool checkChar();
		bool checkSpecial();

		void getLine(); ///< It reads a new line from the iostream and it points @ref pchar to the beginning of that line
		char getNextChar(); ///< Get the next char from the @ref line. If @ref line empty then get new line. It returns '\\0' if we are in the end of the line
		char putBackChar(); ///< Put the char that Scanner::GetNextChar got back to the current line

	//===================================================================================================================================
	// ascii stuff                                                                                                                      =
	//===================================================================================================================================
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

		static AsciiFlag asciiLookupTable []; ///< The array contains one ascii_code_e for every symbol of the ASCII table
		/// Initializes the asciiLookupTable. It runs only once in the construction of the first Scanner @see Scanner()
		static void initAsciiMap();

		/// To save us from typing for example asciiLookupTable[ (int)'a' ]
		inline AsciiFlag asciiLookup( char ch_ )
		{
			return asciiLookupTable[ (int)ch_ ];
		}

		
	//===================================================================================================================================
	// Token and token info                                                                                                             =
	//===================================================================================================================================
	public:
		/// The TokenCode is an enum that defines the Token type
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
			TC_SCOPERESOLUTION, TC_LSQBRACKET, TC_RSQBRACKET, TC_LPAREN, TC_RPAREN, TC_DOT, TC_POINTERTOMEMBER, TC_LBRACKET, TC_RBRACKET, TC_COMMA,
				TC_PERIOD, TC_UPDOWNDOT, TC_QUESTIONMARK, TC_SHARP,                                                                //0 - 13
			TC_EQUAL, TC_NOTEQUAL, TC_LESS, TC_GREATER, TC_LESSEQUAL, TC_GREATEREQUAL, TC_LOGICALOR, TC_LOGICALAND,            //14 - 21
			TC_PLUS, TC_MINUS, TC_STAR, TC_BSLASH, TC_NOT, TC_BITWISEAND, TC_BITWISEOR, TC_ONESCOMPLEMENT, TC_MOD, TC_XOR,       //22 - 31
			TC_INC, TC_DEC, TC_SHL, TC_SHR,                                                                                //32 - 35
			TC_ASSIGN, TC_ASSIGNADD, TC_ASSIGNSUB, TC_ASSIGNMUL, TC_ASSIGNDIV, TC_ASSIGNMOD, TC_ASSIGNSHL, TC_ASSIGNSHR, TC_ASSIGNAND, TC_ASSIGNXOR,
				TC_ASSIGNOR                                                                                             //36 - 47
		}; // end enum TokenCode

		/// The value of @ref Token::dataType
		enum TokenDataType
		{
			DT_FLOAT,
			DT_INT,
			DT_CHAR,
			DT_STR
		};

		/// Used inside the @ref Token, its a variant that holds the data of the Token
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
				// accessors
				char        getChar()   const { return char_; }  ///< Access the data as C char
				ulong       getInt()    const { return int_; }   ///< Access the data as unsigned int
				float       getFloat()  const { return float_; } ///< Access the data as double
				const char* getString() const { return string; } ///< Access the data as C string
		}; // end class TokenDataVal

		/// The Token class
		struct Token
		{
			friend class Scanner;

			PROPERTY_R( TokenCode, code, getCode ) ///< @ref PROPERTY_R : The first thing you shoud know about a token
			PROPERTY_R( TokenDataType, dataType, getDataType ) ///< @ref PROPERTY_R : Additional info in case @ref code is @ref TC_NUMBER
			PROPERTY_R( TokenDataVal, value, getValue ) ///< @ref PROPERTY_R : A value variant

			private:
				char asString[ MAX_SCRIPT_LINE_LEN ];

			public:
				Token(): code( TC_ERROR ) {}
				Token( const Token& b );

				const char* getString() const { return asString; }
		}; // end class Token

	//===================================================================================================================================
	// reserved words                                                                                                                   =
	//===================================================================================================================================
	protected:
		/// Reserved words like "int" "const" etc. Currently the reserved words list is being populated with dummy data
		struct ResWord
		{
			const char* string;
			TokenCode   code;
		};

		static ResWord rw2 [], rw3 [], rw4 [], rw5 [], rw6 [], rw7 []; ///< Groups of ResWord grouped by the length of the ResWord::string
		static ResWord* rwTable []; ///< The array contains all the groups of ResWord

	//===================================================================================================================================
	// vars                                                                                                                            =
	//===================================================================================================================================
	protected:
		Token crntToken; ///< The current token
		char  line [MAX_SCRIPT_LINE_LEN]; ///< In contains the current line's text
		char* pchar; ///< Points somewhere to @ref line
		int   lineNmbr; ///< The number of the current line
		bool  newlinesAsWhitespace; ///< Treat newlines as whitespace. If false means that the Scanner returns (among others) newline tokens
		/**
		 * Used to keep track of the newlines in multiline comments so we can then return the correct number of newlines in case of
		 * newlinesAsWhitespace is false
		 */
		int   commentedLines;

		// input
		ifstream  inFstream; ///< The file stream. Used if the @ref Scanner is initiated using @ref loadFile
		istream*  inStream; ///< Points to either @ref inFstream or an external std::istream
		char      scriptName[512]; ///< The name of the input stream. Mostly used for the error messages inside the @ref Scanner

	//===================================================================================================================================
	// public funcs                                                                                                                     =
	//===================================================================================================================================
	public:
		/**
		 * The one and only constructor
		 * @param newlinesAsWhitespace If false the Scanner will return a newline @ref Token everytime if finds a newline. True is the default behavior
		 */
		Scanner( bool newlinesAsWhitespace = true );
		~Scanner() { /* The destructor does NOTHING. The class does not make any mem allocations */ }

		bool loadFile( const char* filename ); ///< Load a file to extract tokens
		bool loadIstream( istream& istream_, const char* scriptName_ = "unamed-istream" ); ///< Load a STL istream to extract tokens
		void unload();

		static void   printTokenInfo( const Token& token ); ///< Print info of the given token
		static string getTokenInfo( const Token& token ); ///< Get a string with the info of the given token
		       void   getAllPrintAll(); ///< Extracts all tokens and prints them. Used for debugging

		const Token& getNextToken(); ///< Get the next token from the stream
		const Token& getCrntToken() const { return crntToken; } ///< Accessor for the current token

		const char* getScriptName() const { return scriptName; } ///< Get the name of the stream
		int         getLineNumber() const { return lineNmbr; } ///< Get the current line the Scanner is processing
}; // end class Scanner

#endif
