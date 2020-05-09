#include <memory.h>
#include <string.h>
#include <stdint.h>
#include <string>

class leybuf
{

private:
	char*buffer;
	uint32_t size_of_buffer;
	uint32_t curbyte;
	bool overflowed;
	
public:

	~leybuf();
	leybuf(char*buf, uint32_t size, uint32_t byte = 0);
	


	//status funcs
	bool IsOverflowed();

	//set funcs
	bool SetPos(uint32_t byte);

	//get funcs

	int32_t GetSize();

	int32_t GetPos();

	uint32_t GetNumBytesLeft();

	char* GetData();

	//write
	bool WriteChar(char ch);

	bool WriteString(std::string str, bool removenull=false);
	bool WriteString(const char *str, bool removenull=false);

	int32_t WriteFloat(float num);

	int64_t WriteInt64(int64_t num);
	int32_t WriteInt32(int32_t num);
	int16_t WriteInt16(int16_t num);

	bool WriteBytes(void*src, int32_t num);

	//read
	char ReadChar();

	bool ReadString(std::string& buf);
	bool ReadString(char*buf, uint32_t buflen);
	bool SkipString();

	int64_t ReadInt64();
	int32_t ReadInt32();
	int16_t ReadInt16();
	
	float ReadFloat();
	
	bool ReadBytes(void*buf, uint32_t buflen);

};
