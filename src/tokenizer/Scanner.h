#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <sstream>
#include "common.h"


//=====================================================================================================================================
// Scanner                                                                                                                          =
//=====================================================================================================================================
/**
 * The Scanner loads a file and returns the tokens. The script must be in C++ format. The class does not make any kind of
 * memory allocations so it can be fast.
 */
class Scanner
{
	//===================================================================================================================================
	// private stuff                                                                                                                    =
	//===================================================================================================================================
	protected:
		static const int MAX_SCRIPT_LINE_LEN = 1024; ///< The max allowed length of a script line

		static char eofChar; ///< Special end of file character

		bool checkWord();
		bool checkComment();
		bool checkNumber();
		bool checkString();
		bool checkChar();
		bool checkSpecial();

		void getLine(); ///< Reads a line from the script file
		char getNextChar(); ///< Get the next char from the line_txt. If line_txt empty then get new line. It returns '\0' if we are in the end of the line
		char putBackChar(); ///< Put the char that Scanner::GetNextChar got back to the current line

	//===================================================================================================================================
	// ascii stuff                                                                                                                      =
	//===================================================================================================================================
	protected:
		enum AsciiFlagsE
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

		static uint asciiLookupTable []; ///< The array contains one ascii_code_e for every symbol of the ASCII table
		/// Initializes the asciiLookupTable. It runs only once in the construction of the first Scanner. See Scanner()
		static void initAsciiMap();

		/// To save us from typing for example asciiLookupTable[ (int)'a' ]
		inline uint asciiLookup( char ch_ )
		{
			return asciiLookupTable[ (int)ch_ ];
		}

		
	//===================================================================================================================================
	// token types                                                                                                                      =
	//===================================================================================================================================
	public:
		/// The TokenCode is an enum that defines the Token
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
		};

		/// TokenDataType
		enum TokenDataType
		{
			DT_FLOAT,
			DT_INT,
			DT_CHAR,
			DT_STR
		};

		/// TokenDataVal
		union TokenDataVal
		{
			char   char_;
			ulong  int_;
			double float_;
			char*  string; ///< points to Token::as_string if the token is string
		};

		/// The Token class
		struct Token
		{
			TokenCode      code;
			TokenDataType  type;
			TokenDataVal   value;

			char as_string[ MAX_SCRIPT_LINE_LEN ];

			Token(): code( TC_ERROR ) {}
			Token( const Token& b );
		};

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
		static ResWord* rw_table []; ///< The array contains all the groups of ResWord

	//===================================================================================================================================
	// vars                                                                                                                            =
	//===================================================================================================================================
	protected:
		Token crntToken; ///< The current token
		char  line [MAX_SCRIPT_LINE_LEN]; ///< In contains the current line's text
		char* pchar; ///< Points to line_txt
		int   lineNmbr; ///< The number of the current line
		bool  newlinesAsWhitespace; ///< Treat newlines as whitespace. This means that we extract new token for every line
		/**
		 * Used to keep track of the newlines in multiline comments so we can return the correct number of newlines in case of 
		 * newlines_as_whitespace is false
		 */
		int     commentedLines;

		// input
		fstream      inFstream;
		iostream*    inStream; ///< Points to either in_fstream or an external std::iostream
		char         scriptName[64]; ///< The name of the input stream. Mostly used for error handling

	//===================================================================================================================================
	// public funcs                                                                                                                     =
	//===================================================================================================================================
	public:
		Scanner( bool newlinesAsWhitespace = true );
		~Scanner() { /* The destructor does NOTHING. The class does not make any mem allocations */ }

		bool loadFile( const char* filename ); ///< Load a file to extract tokens
		bool loadIoStream( iostream* iostream_, const char* scriptName_ = "unamed-iostream" ); ///< Load a STL iostream to extract tokens
		void unload();

		static void   printTokenInfo( const Token& token ); ///< Print info of the given token
		static string getTokenInfo( const Token& token ); ///< Get a string with the info of the given token
		       void   getAllPrintAll();

		const Token& getNextToken(); ///< Get the next token from the file
		const Token& getCrntToken() const { return crntToken; } ///< Accessor for the current token

		const char* getScriptName() const { return scriptName; } ///< Accessor for member variable
		int         getLineNmbr() const { return lineNmbr; } ///< Accessor for member variable
};

#endif
