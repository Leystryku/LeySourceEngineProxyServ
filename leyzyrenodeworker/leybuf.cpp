#include "leybuf.h"

leybuf::~leybuf()
{
	buffer = 0;
	size_of_buffer = 0;
	curbyte = 0;
}

leybuf::leybuf(char*buf, uint32_t size, uint32_t byte = 0)
{
	buffer = buf;
	size_of_buffer = size;
	curbyte = byte;
	overflowed = false;
}

//status funcs
bool leybuf::IsOverflowed()
{
	if (curbyte >= size_of_buffer)
		overflowed = true;

	return overflowed;
}

//set funcs
bool leybuf::SetPos(uint32_t byte)
{
	if (byte > size_of_buffer)
	{
		overflowed = true;
		return false;
	}

	curbyte = byte;
	return true;
}

//get funcs

int32_t leybuf::GetSize()
{
	return size_of_buffer;
}

int32_t leybuf::GetPos()
{
	return curbyte;
}

uint32_t leybuf::GetNumBytesLeft()
{
	return size_of_buffer - curbyte;
}

char* leybuf::GetData()
{
	return buffer;
}

//write
bool leybuf::WriteChar(char ch)
{
	if(curbyte + 1 > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return false;

	buffer[curbyte] = ch;
	curbyte++;


	return true;
}

bool leybuf::WriteString(std::string str, bool removenull=false)
{
	const char *cstr = str.c_str();

	return WriteString(cstr, removenull);
}


bool leybuf::WriteString(const char *str, bool removenull=false)
{
	if(!str)
	{
		return false;
	}

	if(curbyte + strlen(str) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return false;
	
	if (removenull)
	{
		for (uint32_t i = 0; i < strlen(str); i++)
		{
			buffer[curbyte] = str[i];
			curbyte++;
		}

		return true;
	}
	strcpy(buffer + curbyte, str);
	curbyte += strlen(str)+1;


	return true;
}

int32_t leybuf::WriteFloat(float num)
{

	if(curbyte + sizeof(float) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	*(float*)( buffer + curbyte ) = num;

	curbyte += 4;

	return num;
}

int64_t leybuf::WriteInt64(int64_t num)
{

	if(curbyte + sizeof(int64_t) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	*(int64_t*)( buffer + curbyte ) = num;

	curbyte += 8;


	return num;
}

int32_t leybuf::WriteInt32(int32_t num)
{

	if(curbyte + sizeof(int32_t) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	*(int32_t*)( buffer + curbyte ) = num;

	curbyte += 4;


	return num;
}

int16_t leybuf::WriteInt16(int16_t num)
{

	if(curbyte + sizeof(int16_t) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	*(int16_t*)( buffer + curbyte ) = num;

	curbyte += 2;

	return num;
}

bool leybuf::WriteBytes(void*src, int32_t num)
{

	if(curbyte + num > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return false;

	memcpy(buffer + curbyte, src,  num);
	curbyte += num;


	return true;
}

//read
char leybuf::ReadChar()
{

	if(curbyte + sizeof(char) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	char ret = buffer[curbyte];
	curbyte++;


	return ret;
}

bool leybuf::ReadString(std::string &buf)
{
	if(curbyte + buf.length() > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return false;

	char* curchar = (char*)buffer + curbyte;

	
	buf.clear();
	buf.append(curchar);

	curbyte += strlen(buffer+curbyte)+1;

	return true;

}
bool leybuf::ReadString(char*buf, uint32_t buflen)
{

	bool foundzero = false;
	for(int i=0;i<size_of_buffer - curbyte;i++)
	{
		char *read = buffer + curbyte + i;
		if(*read == 0)
		{
			foundzero = true;
		}
	}

	if(!foundzero)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return false;


	if (strlen(buffer + curbyte) > buflen)
		return false;

	strcpy(buf, buffer+curbyte);
	curbyte += strlen(buffer+curbyte)+1;

	return true;
}

bool leybuf::SkipString()
{
	if (IsOverflowed())
		return false;

	curbyte += strlen(buffer+curbyte)+1;

	if (curbyte > size_of_buffer)
		curbyte = size_of_buffer;

	if (IsOverflowed())
		return false;

	return true;
}

int16_t leybuf::ReadInt16()
{

	if(curbyte + sizeof(int16_t) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	int16_t ret = *(int16_t*)( buffer + curbyte );

	curbyte+=2;

	return ret;
}

int32_t leybuf::ReadInt32()
{
	if(curbyte + sizeof(int32_t) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	int32_t ret = *(int32_t*)( buffer + curbyte );

	curbyte+=4;

	return ret;
}

int64_t leybuf::ReadInt64()
{
	if(curbyte + sizeof(int64_t) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	int64_t ret = *(int64_t*)( buffer + curbyte );


	curbyte+=8;

	return ret;
}

float leybuf::ReadFloat()
{

	if(curbyte + sizeof(float) > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return 0;

	float ret = *(float*)( buffer + curbyte );


	curbyte+=4;


	
	return ret;
}

bool leybuf::ReadBytes(void*buf, uint32_t buflen)
{

	if(curbyte + buflen > size_of_buffer)
	{
		overflowed = true;
	}

	if (IsOverflowed())
		return false;
	
	uint32_t maxr = buflen;
	
	memcpy(buf, buffer+curbyte, buflen);

	return true;
}