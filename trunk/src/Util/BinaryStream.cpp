#include <cstring>
#include <boost/lexical_cast.hpp>
#include "BinaryStream.h"


//======================================================================================================================
// read32bitNumber                                                                                                     =
//======================================================================================================================
template<typename Type>
Type BinaryStream::read32bitNumber()
{
	// Read data
	uchar c[4];
	read((char*)c, sizeof(c));
	if(fail())
	{
		throw EXCEPTION("Failed to read stream");
	}

	// Copy it
	ByteOrder mbo = getMachineByteOrder();
	Type out;
	if(mbo == byteOrder)
	{
		memcpy(&out, c, 4);
	}
	else if(mbo == BO_BIG_ENDIAN && byteOrder == BO_LITTLE_ENDIAN)
	{
		out = (c[0] | (c[1] << 8) | (c[2] << 16) | (c[3] << 24));
	}
	else
	{
		out = ((c[0] << 24) | (c[1] << 16) | (c[2] << 8) | c[3]);
	}
	return out;
}


//======================================================================================================================
// readUint                                                                                                            =
//======================================================================================================================
uint BinaryStream::readUint()
{
	return read32bitNumber<uint>();
}


//======================================================================================================================
// readFloat                                                                                                           =
//======================================================================================================================
float BinaryStream::readFloat()
{
	return read32bitNumber<float>();
}


//======================================================================================================================
// readString                                                                                                          =
//======================================================================================================================
std::string BinaryStream::readString()
{
	uint size;
	try
	{
		size = readUint();
	}
	catch(Exception& e)
	{
		throw EXCEPTION("Failed to read size");
	}

	const uint buffSize = 1024;
	if((size + 1) > buffSize)
	{
		throw EXCEPTION("String bigger than default");
	}
	char buff[buffSize];
	read(buff, size);
	if(fail())
	{
		throw EXCEPTION("Failed to read " + boost::lexical_cast<std::string>(size) + " bytes");
	}
	buff[size] = '\0';

	return std::string(buff);
}


//======================================================================================================================
// getMachineByteOrder                                                                                                 =
//======================================================================================================================
BinaryStream::ByteOrder BinaryStream::getMachineByteOrder()
{
	int num = 1;
	if(*(char*)&num == 1)
	{
		return BO_LITTLE_ENDIAN;
	}
	else
	{
		return BO_BIG_ENDIAN;
	}
}
