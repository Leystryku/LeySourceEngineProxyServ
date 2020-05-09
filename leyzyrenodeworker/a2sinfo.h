#include <stdint.h>
#include <string>

struct a2sinfo
{
	uint8_t header;
	uint8_t protocol;
	std::string name;
	std::string map;
	std::string folder;
	std::string game;
	int16_t appid;
	
	uint8_t players;
	uint8_t maxplayers;
	uint8_t bots;

	uint8_t servertype;
	uint8_t enviroment;

	uint8_t visibility;
	uint8_t vac;

	std::string version;
	
	uint8_t edf;
	
	uint16_t edf_gameport;
	uint64_t edf_serversid;

	uint16_t edf_sourcetv_port;
	std::string edf_sourcetv_name;

	std::string edf_keywords;

	uint64_t edf_servergameid;
};