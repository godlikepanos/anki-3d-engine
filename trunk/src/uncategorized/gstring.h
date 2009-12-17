#ifndef _GSTRING_H_
#define _GSTRING_H_

#include <iostream>
#include <cstdlib>
#include <string.h>

using namespace std;


class string_t
{
	private:
		char* data;
		bool const_data;
		uint length;
		
	public:
		// friends
		friend ostream& operator <<( ostream& output, const string_t& str );
		friend string_t operator +( const string_t& a, const string_t& b );
		friend string_t operator +( const char* a, const string_t& b );
		friend string_t operator +( char a, const string_t& b );
		friend string_t operator +( const string_t& a, const char* b );
		friend string_t operator +( const string_t& a, char b );

		// constructor []
		string_t(): data(NULL), const_data(true), length(0) {}
				
		// constructor [const char*] 
		string_t( const char* pchar )
		{
			data = const_cast<char*>(pchar);
			const_data = true;
			length = strlen(pchar);
			DEBUG_ERR( Length()<1 );
		}		
		
		// constructor [char*]
		string_t( char* pchar )
		{
			length = strlen( pchar );
			const_data = false;
			DEBUG_ERR( Length()<1 );
			data = (char*) malloc( (Length()+1)*sizeof(char) );
			memcpy( data, pchar, Length()+1 );
		}		
		
		// constructor [string_t]
		string_t( const string_t& b )
		{
			const_data = b.const_data;
			length = b.Length();
			if( b.const_data )
				data = b.data;
			else
			{
				data = (char*) malloc( (Length()+1)*sizeof(char) );
				memcpy( data, b.data, Length()+1 );
			}
		}	
		
		// constructor [char]
		string_t( char char_ )
		{
			DEBUG_ERR( char_ == '\0' );
			data = (char*) malloc( 2*sizeof(char) );
			data[0] = char_;
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
		
		// = [string_t]
		string_t& operator =( const string_t& b )
		{
			if( data!=NULL && !const_data )
				free( data );
			const_data = b.const_data;
			length = b.Length();
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
			if( data!=NULL && !const_data )
				free( data );
			length = strlen( pchar );
			const_data = false;
			DEBUG_ERR( Length()<1 );
			data = (char*) malloc( (Length()+1)*sizeof(char) );
			memcpy( data, pchar, Length()+1 );
			return (*this);
		}
		
		// = [const char*]
		string_t& operator =( const char* pchar )
		{
			if( data!=NULL && !const_data )
				free( data );
			data = const_cast<char*>(pchar);
			const_data = true;
			length = strlen(pchar);
			DEBUG_ERR( Length()<1 );
			return (*this);
		}
		
		// = [char]
		string_t& operator =( char char_ )
		{
			if( data!=NULL && !const_data )
				free( data );
			DEBUG_ERR( char_ == '\0' );
			data = (char*) malloc( 2*sizeof(char) );
			data[0] = char_;
			data[1] = '\0';
			return (*this);
		}
		
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
		
		// Clear
		void Clear()
		{
			DEBUG_ERR( (data==NULL && length!=0) || (data!=NULL && length==0) );
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
			DEBUG_ERR( data == NULL );
			return data;
		}
		
		// Length
		uint Length() const
		{
			DEBUG_ERR( data!=NULL && length!=strlen(data) );
			return length;
		}
		
		// Find
		uint Find( char c, uint pos = 0 ) const
		{
			DEBUG_ERR( pos >= length ); // not my bug
			char* pc = data + pos;
			while( *pc!=c && *pc!='\0' )
			{
				++pc;
			}
			return pc - data;
		}
};


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


/// For cout support
inline ostream& operator <<( ostream& output, const string_t& str ) 
{
	output << str.data;
	return output;
}

#endif
