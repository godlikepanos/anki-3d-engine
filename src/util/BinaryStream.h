#ifndef BINARY_STREAM_H
#define BINARY_STREAM_H

#include <iostream>
#include "util/Exception.h"
#include "util/StdTypes.h"


/// Read from binary streams. You can read/write data as if it is an iostream but it also contains methods for
/// reading/writing binary data
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
		BinaryStream(std::streambuf* sb, ByteOrder byteOrder = BO_LITTLE_ENDIAN);

		/// Read unsigned int (32bit)
		/// @exception Exception
		uint readUint();

		/// Read float (32bit)
		/// @exception Exception
		float readFloat();

		/// Read a string. It reads the size as an unsigned int and then it reads the characters
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


inline BinaryStream::BinaryStream(std::streambuf* sb, ByteOrder byteOrder_):
	std::iostream(sb),
	byteOrder(byteOrder_)
{}


#endif
