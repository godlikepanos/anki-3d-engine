#ifndef _GSTRING_H_
#define _GSTRING_H_

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <climits>
#include <cmath>
#include "common.h"

using namespace std;

/// class string_t
class string_t
{
	private:
		// friends
		friend ostream& operator <<( ostream& output, const string_t& str );
		friend string_t operator +( const string_t& a, const string_t& b );
		friend string_t operator +( const char* a, const string_t& b );
		friend string_t operator +( char a, const string_t& b );
		friend string_t operator +( const string_t& a, const char* b );
		friend string_t operator +( const string_t& a, char b );
		friend bool operator ==( const string_t& a, const string_t& b );
		friend bool operator ==( const string_t& a, const char* b );
		friend bool operator ==( const char* a, const string_t& b );
		
		// data
		char* data;
		bool const_data; ///< false means that we have allocated memory
		uint length;
		
		// private stuff
		void Init( float d )
		{
			char pc [256];
			sprintf( pc, "%f", d );
			length = strlen( pc );
			const_data = false;
			data = (char*) malloc( (length+1)*sizeof(char) );
			memcpy( data, pc, length+1 );
		}
		
	public:
		static const uint npos = UINT_MAX;  // ToDo: change it when C++0x becomes standard
		
		// constructor []
		string_t(): data(NULL), const_data(true), length(0) {}
				
		// constructor [const char*] 
		string_t( const char* pchar )
		{
			data = const_cast<char*>(pchar);
			const_data = true;
			length = strlen(pchar);
		}		
		
		// constructor [char*]
		string_t( char* pchar )
		{
			length = strlen( pchar );
			const_data = false;
			data = (char*) malloc( (length+1)*sizeof(char) );
			memcpy( data, pchar, length+1 );
		}		
		
		// constructor [string_t]
		string_t( const string_t& b )
		{
			const_data = b.const_data;
			length = b.length;
			if( b.const_data )
				data = b.data;
			else
			{
				data = (char*) malloc( (length+1)*sizeof(char) );
				memcpy( data, b.data, length+1 );
			}
		}	
		
		// constructor [char]
		string_t( char c )
		{
			DEBUG_ERR( c == '\0' );
			length = 1;
			data = (char*) malloc( 2*sizeof(char) );
			const_data = false;
			data[0] = c;
			data[1] = '\0';
		}
		
		// destructor
		virtual ~string_t()
		{
			if( length!=0 && !const_data )
				free( data );
		}
		
		// []
		const char& operator []( uint i ) const { DEBUG_ERR(i>=length); return data[i]; }
		char& operator []( uint i ) { DEBUG_ERR(i>=length); return data[i]; }
		
		
		//================================================================================================================================
		// operator =                                                                                                                    =
		//================================================================================================================================
		
		// = [string_t]
		string_t& operator =( const string_t& b )
		{
			if( !const_data ) free( data );
			const_data = b.const_data;
			length = b.length;
			if( b.const_data )
				data = b.data;
			else
			{
				data = (char*) malloc( (length+1)*sizeof(char) );
				memcpy( data, b.data, length+1 );
			}
			return (*this);
		}
			
		// = [char*]
		string_t& operator =( char* pchar )
		{
			if( !const_data ) free( data );
			length = strlen( pchar );
			const_data = false;
			data = (char*) malloc( (length+1)*sizeof(char) );
			memcpy( data, pchar, length+1 );
			return (*this);
		}
		
		// = [const char*]
		string_t& operator =( const char* pchar )
		{
			if( !const_data ) free( data );
			data = const_cast<char*>(pchar);
			const_data = true;
			length = strlen(pchar);
			return (*this);
		}
		
		// = [char]
		string_t& operator =( char c )
		{
			if( !const_data ) free( data );
			DEBUG_ERR( c == '\0' );
			data = (char*) malloc( 2*sizeof(char) );
			data[0] = c;
			data[1] = '\0';
			return (*this);
		}
		
		
		//================================================================================================================================
		// operator +=                                                                                                                   =
		//================================================================================================================================
		
		// += [string_t]
		string_t& operator +=( const string_t& str )
		{
			if( const_data )
			{
				char* data_ = (char*) malloc( (length+str.length+1)*sizeof(char) );
				memcpy( data_, data, length );
				data = data_;
				const_data = false;
			}
			else
			{
				data = (char*) realloc( data, (length+str.length+1)*sizeof(char) );
			}
			memcpy( data+length, str.data, (str.length+1)/sizeof(char) );
			length += str.length;
			return (*this);
		}
			
		// += [char*]
		string_t& operator +=( const char* pchar )
		{
			uint pchar_len = strlen( pchar );
			if( const_data )
			{
				char* data_ = (char*) malloc( (length+pchar_len+1)*sizeof(char) );
				memcpy( data_, data, length );
				data = data_;
				const_data = false;
			}
			else
			{
				data = (char*) realloc( data, (length+pchar_len+1)*sizeof(char) );
			}
			memcpy( data+length, pchar, (pchar_len+1)/sizeof(char) );
			length += pchar_len;
			return (*this);
		}
		
		// += [char]
		string_t& operator +=( const char char_ )
		{
			if( const_data )
			{
				char* data_ = (char*) malloc( (length+2)*sizeof(char) );
				memcpy( data_, data, length );
				data = data_;
				const_data = false;
			}
			else
			{
				data = (char*) realloc( data, (length+2)*sizeof(char) );
			}
			data[length] = char_;
			data[length+1] = '\0';
			++length;
			return (*this);
		}
		
		
		//================================================================================================================================
		// misc                                                                                                                          =
		//================================================================================================================================
		
		// Convert
		static string_t Convert( int i )
		{
			string_t out;
			if( i == 0 )
			{
				out.data = "0";
				out.const_data = true;
				out.length = 1;
			}
			else
			{
				char cv [256];
				const size_t cv_size = sizeof(cv)/sizeof(char); 
				char* pc = &cv[ cv_size ];
				int ii = (i<0) ? -i : i;
				while( ii != 0 )
				{
					int mod = ii%10;
					ii /= 10;
					--pc;
					*pc = ('0' + mod);
				}
				if( i < 0 )
				{
					--pc;
					*pc = '-';
				}
				out.length = cv + cv_size - pc;
				DEBUG_ERR( out.length >= sizeof(cv)/sizeof(char) );
				out.const_data = false;
				out.data = (char*) malloc( (out.length+1)*sizeof(char) );
				memcpy( out.data, pc, out.length );
				out.data[out.length] = '\0';
			}
			return out;
		}
		
		// Convert [float]
		static string_t Convert( float f )
		{
			string_t out;
			char pc [256];
			sprintf( pc, "%f", f );
			out.length = strlen( pc );
			out.const_data = false;
			out.data = (char*) malloc( (out.length+1)*sizeof(char) );
			memcpy( out.data, pc, out.length+1 );
			return out;
		}
		
		// Convert [double]
		static string_t Convert( double d )
		{
			return Convert( (float)d );
		}
		
		// Clear
		void Clear()
		{
			DEBUG_ERR( (data==NULL && length!=0) || (data!=NULL && length==0) ); // not my bug
			if( data != NULL )
			{
				if( !const_data )
					free( data );
				const_data = true;
				length = 0;
				data = NULL;
			}
		}
			
		// CStr
		const char* CStr() const
		{
			DEBUG_ERR( data == NULL ); // not my bug
			return data;
		}
		
		// Length
		uint Length() const
		{
			DEBUG_ERR( data!=NULL && length!=strlen(data) );
			return length;
		}
		
		// Find [char]
		uint Find( char c, uint pos = 0 ) const
		{
			DEBUG_ERR( pos >= length ); // not my bug
			char* pc = data + pos;
			while( *pc!=c && *pc!='\0' )
			{
				++pc;
			}
			uint diff = pc - data; 
			return (diff == length) ? npos : diff;
		}
		
		// RFind [char]
		uint RFind( char c, uint pos = npos ) const
		{
			DEBUG_ERR( pos >= length && pos!=npos ); // not my bug
			if( pos==npos ) pos = length-1;
			char* pc = data + pos;
			while( pc!=data-1 && *pc!=c )
			{
				--pc;
			}
			return (pc == data-1) ? npos : pc - data;
		}
		
		// Find [string_t]
		uint Find( const string_t& str, uint pos = 0 ) const
		{
			DEBUG_ERR( pos >= length ); // not my bug
			char* pc = strstr( data+pos, str.data );
			return (pc == NULL) ? npos : pc-data;
		}
		
		// Find [char*]
		uint Find( const char* pc, uint pos = 0 ) const
		{
			DEBUG_ERR( pos >= length ); // not my bug
			char* pc_ = strstr( data+pos, pc );
			return (pc_ == NULL) ? npos : pc_-data;
		}
};


//================================================================================================================================
// operator +                                                                                                                    =
//================================================================================================================================

/// string_t + string_t
inline string_t operator +( const string_t& a, const string_t& b )
{
	string_t out;
	out.const_data = false;
	out.data = (char*) malloc( (a.length+b.length+1)*sizeof(char) );
	memcpy( out.data, a.data, a.length );
	memcpy( out.data+a.length, b.data, b.length+1 );
	out.length = a.length+b.length;
	return out;
}

/// char* + string_t
inline string_t operator +( const char* a, const string_t& b )
{
	DEBUG_ERR( strlen(a)<1 || b.length<1 ); // not my bug
	string_t out;
	uint a_length = strlen( a );
	out.length = a_length+b.length;
	out.const_data = false;
	out.data = (char*) malloc( (out.length+1)*sizeof(char) );
	memcpy( out.data, a, a_length );
	if( b.length > 0 )
		memcpy( out.data+a_length, b.data, b.length+1 );
	else
		out.data[out.length] = '\0';
	return out;
}

/// char + string_t
inline string_t operator +( char a, const string_t& b )
{
	string_t out;
	out.const_data = false;
	out.data = (char*) malloc( (b.length+2)*sizeof(char) );
	out.data[0] = a;
	memcpy( out.data+1, b.data, b.length+1 );
	out.length = 1+b.length;
	return out;
}

/// string_t + char*
inline string_t operator +( const string_t& a, const char* b )
{
	DEBUG_ERR( a.length<1 || strlen(b)<1 ); // not my bug
	string_t out;
	uint b_length = strlen( b );
	out.const_data = false;
	out.length = a.length+b_length;
	out.data = (char*) malloc( (out.length+1)*sizeof(char) );
	memcpy( out.data, a.data, a.length );
	memcpy( out.data+a.length, b, b_length+1 );
	return out;
}

/// string_t + char
inline string_t operator +( const string_t& a, char b )
{
	DEBUG_ERR( a.length<1 ); // not my bug
	string_t out;
	out.const_data = false;
	out.data = (char*) malloc( (a.length+2)*sizeof(char) );
	memcpy( out.data, a.data, a.length );
	out.data[a.length] = b;
	out.data[a.length+1] = '\0';
	out.length = a.length+1;
	return out;
}


//================================================================================================================================
// operator ==                                                                                                                   =
//================================================================================================================================

/// string_t == string_t
inline bool operator ==( const string_t& a, const string_t& b )
{
	return strcmp( a.data, b.data ) == 0;
}

/// string_t == char*
inline bool operator ==( const string_t& a, const char* b )
{
	return strcmp( a.data, b ) == 0;
}

/// char* == string_t
inline bool operator ==( const char* a, const string_t& b )
{
	return strcmp( a, b.data ) == 0;
}


//================================================================================================================================
// misc                                                                                                                          =
//================================================================================================================================
/// For cout support
inline ostream& operator <<( ostream& output, const string_t& str ) 
{
	output << str.data;
	return output;
}

#endif
