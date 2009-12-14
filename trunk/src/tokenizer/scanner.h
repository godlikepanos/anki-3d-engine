#ifndef _SCANNER_H_
#define _SCANNER_H_

#include <sstream>
#include "common.h"


//=====================================================================================================================================
// scanner_t                                                                                                                          =
//=====================================================================================================================================
/**
 * The scanner_t loads a file and returns the tokens. The script must be in C++ format. The class does not make any kind of
 * memory allocations so it can be fast.
 */
class scanner_t
{
	//===================================================================================================================================
	// private stuff                                                                                                                    =
	//===================================================================================================================================
	protected:
		static const int MAX_SCRIPT_LINE_LEN = 1024; ///< The max allowed length of a script line

		static char eof_char; ///< Special end of file character

		bool CheckWord();
		bool CheckComment();
		bool CheckNumber();
		bool CheckString();
		bool CheckChar();
		bool CheckSpecial();

		void GetLine(); ///< Reads a line from the script file
		char GetNextChar(); ///< Gets a char from the current script line
		char PutBackChar(); ///< Put the char that scanner_t::GetNextChar got back to the current line

	//===================================================================================================================================
	// ascii stuff                                                                                                                      =
	//===================================================================================================================================
	protected:
		enum ascii_flags_e
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

		static uint ascii_lookup_table []; ///< The array contains one ascii_code_e for every symbol of the ASCII table
		/// Initializes the ascii_lookup_table. It runs only once in the construction of the first scanner_t. See scanner_t()
		static void InitAsciiMap();

		/// To save us from typing for example ascii_lookup_table[ (int)'a' ]
		inline uint AsciiLookup( char ch_ )
		{
			return ascii_lookup_table[ (int)ch_ ];
		}

		
	//===================================================================================================================================
	// token types                                                                                                                      =
	//===================================================================================================================================
	public:
		/// The token_code_e is an enum that defines the token_t
		enum token_code_e
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

		/// token_data_type_e
		enum token_data_type_e
		{
			DT_FLOAT,
			DT_INT,
			DT_CHAR,
			DT_STR
		};

		/// token_data_val_u
		union token_data_val_u
		{
			char   char_;
			ulong  int_;
			double float_;
			char*  string; ///< points to token_t::as_string if the token is string
		};

		/// The token_t class
		struct token_t
		{
			token_code_e       code;
			token_data_type_e  type;
			token_data_val_u   value;

			char as_string[ MAX_SCRIPT_LINE_LEN ];

			token_t(): code( TC_ERROR ) {}
			token_t( const token_t& b );
		};

	//===================================================================================================================================
	// reserved words                                                                                                                   =
	//===================================================================================================================================
	protected:
		/// Reserved words like "int" "const" etc. Currently the reserved words list is being populated with dummy data
		struct res_word_t
		{
			const char*  string;
			token_code_e code;
		};

		static res_word_t rw2 [], rw3 [], rw4 [], rw5 [], rw6 [], rw7 []; ///< Groups of res_word_t grouped by the length of the res_word_t::string
		static res_word_t* rw_table []; ///< The array contains all the groups of res_word_t

	//===================================================================================================================================
	// vars                                                                                                                            =
	//===================================================================================================================================
	protected:
		token_t crnt_token; ///< The current token
		char    line_txt [MAX_SCRIPT_LINE_LEN]; ///< In contains the current line's text
		char*   pchar; ///< Points to line_txt
		int     line_nmbr; ///< The number of the current line
		bool    newlines_as_whitespace; ///< Treat newlines as whitespace
		/*
		 * Used to keep track of the newlines in multiline comments so we can return the correct number of newlines in case of 
		 * newlines_as_whitespace is false
		 */
		int     commented_lines;  

		// input
		stringstream in_sstream;
		fstream      in_fstream;
		iostream*    in_stream; ///< points to either in_fstream or in_sstream
		char         script_name[64]; ///< The name of the input stream. Mostly used for error handling

	//===================================================================================================================================
	// public funcs                                                                                                                     =
	//===================================================================================================================================
	public:
		scanner_t( bool newlines_as_whitespace = true );
		~scanner_t() { /* The destructor does NOTHING. The class does not make any mem allocations */ }

		bool LoadFile( const char* filename ); ///< Load a file to extract tokens
		bool LoadString( const string& str, const char* script_name_ = "*string*" );
		void Unload();

		static void   PrintTokenInfo( const token_t& token ); ///< Print info of the given token
		static string GetTokenInfo( const token_t& token ); ///< Get a string with the info of the given token
		       void   GetAllPrintAll();

		const token_t& GetNextToken(); ///< Get the next token from the file
		const token_t& GetCrntToken() const { return crnt_token; } ///< Accessor for the current token

		const char* GetScriptName() const { return script_name; } ///< Accessor for member variable
		int         GetLineNmbr() const { return line_nmbr; } ///< Accessor for member variable
};

#endif
