#include <stdint.h>

struct SourceSplitPacket
{
	int32_t id;
	int8_t total;
	int8_t number;
	int16_t packetsize;
	
	char msg[1400];
};

