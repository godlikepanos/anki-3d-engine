#include <cstring>
#include "BinaryStream.h"


using namespace std;


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
		THROW_EXCEPTION("Failed to read stream");
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
string BinaryStream::readString()
{
	uint size;
	try
	{
		size = readUint();
	}
	catch(Exception& e)
	{
		THROW_EXCEPTION("Failed to read size");
	}

	const uint buffSize = 1024;
	if((size + 1) > buffSize)
	{
		THROW_EXCEPTION("String bigger than default");
	}
	char buff[buffSize];
	read(buff, size);
	if(fail())
	{
		stringstream ss;
		ss << "Failed to read " << size << " bytes";
		THROW_EXCEPTION(ss.str().c_str());
	}
	buff[size] = '\0';

	return string(buff);
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
