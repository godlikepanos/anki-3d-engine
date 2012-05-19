#include "anki/util/BinaryStream.h"
#include <cstring>
#include <string>

namespace anki {

//==============================================================================
template<typename Type>
Type BinaryStream::read32bitNumber()
{
	// Read data
	uint8_t c[4];
	read((char*)c, sizeof(c));
	if(fail())
	{
		throw ANKI_EXCEPTION("Failed to read stream");
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

//==============================================================================
uint32_t BinaryStream::readUint()
{
	return read32bitNumber<uint32_t>();
}

//==============================================================================
float BinaryStream::readFloat()
{
	return read32bitNumber<float>();
}

//==============================================================================
std::string BinaryStream::readString()
{
	uint32_t size;
	try
	{
		size = readUint();
	}
	catch(Exception& e)
	{
		throw ANKI_EXCEPTION("Failed to read size");
	}

	const uint32_t buffSize = 1024;
	if((size + 1) > buffSize)
	{
		throw ANKI_EXCEPTION("String bigger than temp buffer");
	}
	
	char buff[buffSize];
	read(buff, size);
	
	if(fail())
	{
		throw ANKI_EXCEPTION("Failed to read " 
			+ std::to_string(size) + " bytes");
	}
	buff[size] = '\0';

	return std::string(buff);
}

//==============================================================================
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

} // end namespace
