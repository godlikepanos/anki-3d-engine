#ifndef ANKI_UTIL_BINARY_STREAM_H
#define ANKI_UTIL_BINARY_STREAM_H

#include "anki/util/Exception.h"
#include "anki/util/StdTypes.h"
#include <iostream>

namespace anki {

/// Read from binary streams. You can read/write data as if it is an iostream 
/// but it also contains methods for reading/writing binary data
class BinaryStream: public std::iostream
{
public:
	/// The 2 available byte orders
	enum ByteOrder
	{
		BO_LITTLE_ENDIAN, ///< The default
		BO_BIG_ENDIAN
	};

	/// The one and only constructor
	/// @param sb An std::streambuf for in/out
	/// @param byteOrder The stream's byte order
	BinaryStream(std::streambuf* sb,
		ByteOrder byteOrder_ = BO_LITTLE_ENDIAN)
		: std::iostream(sb), byteOrder(byteOrder_)
	{}

	~BinaryStream();

	/// Read unsigned int (32bit)
	/// @exception Exception
	U32 readUint();

	/// Read float (32bit)
	/// @exception Exception
	F32 readFloat();

	/// Read a string. It reads the size as an unsigned int and then it
	/// reads the characters
	/// @exception Exception
	std::string readString();

	/// Get machine byte order
	/// @return The byte order of the current running platform
	static ByteOrder getMachineByteOrder();

private:
	ByteOrder byteOrder;

	/// A little hack so we dont write duplicate code
	template<typename Type>
	Type read32bitNumber();
};

} // end namespace

#endif
