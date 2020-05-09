#include <string>
#include <stdint.h>

class WorkerData
{
public:

	WorkerData();

	std::string str_srcipandport;

	std::string str_srcip;
	uint32_t iip;
	std::string str_srcport;
	uint16_t isrcport;
	
	uint16_t a2sport;
	uint16_t proxyport;
	uint16_t gameport;

	std::string password;
	uint64_t ipassword;

	uint8_t extraplayers;
	uint16_t sidoffset;
	
	std::string customname;
	std::string customip;

	bool nomaster;
	bool sharedport;
	bool a2signore;
};