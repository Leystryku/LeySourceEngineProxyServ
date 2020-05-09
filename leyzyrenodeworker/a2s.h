//#define A2S_MAXDATASIZE 4096
#define A2S_MAXDATASIZE 200000
#define A2S_MASTERSERVERCOUNT 6

#define A2S_REFRESHTIMEMASTER 30000 // every 30 seconds we refresh the masterserv
#define A2S_REFRESHTIME 20000 // every 20 seconds we refresh the a2s_info
#define A2S_REFRESHTIMEPLY  59000 // every 59 seconds we refresh a2s_players

#define A2S_MAXPATHSIZE 256 // max size for folders

#include <chrono>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <stdint.h>

#include "leynet.h"
#include "a2sinfo.h"
#include "a2sply.h"
#include "proxyserver.h"
#include "splitpacket.h"

using std::chrono::steady_clock;

class WorkerData;
class leybuf;


struct node
{

	uint8_t id;
	uint32_t ip;
	double longitude;
	double latitude;
	bool ismaster;
	bool isournode;
	char name[256];

};

class A2SServer
{
public:
	A2SServer(WorkerData *setworker);
	~A2SServer();

	void WriteProxyHeader(leybuf *towrite, uint32_t numip, uint16_t port, uint8_t flags);

	void OnReceiveNoData();
	void OnReceiveData();

	void OnReceiveMasterServerData();

	int32_t OnReceiveSourceServerA2SData();
	void OnReceiveSourceServerA2SInfo();
	void OnReceiveSourceServerA2SPlayers();
	void OnReceiveSourceServerA2SPlyChallenge();
	
	int32_t OnReceiveClientData();
	void OnReceiveClientReqA2SInfo();
	void OnReceiveClientReqA2SPlayers();

	void RequestSourceServerInfo();
	void RequestSourceServerA2SPlayers();
	void RequestSourceServerA2SPlayersWChan();
	
	void RequestMasterServerChallenge();

	void RequestMasterDataIfNeeded();
	void RequestInfoDataIfNeeded();
	void RequestPlayerDataIfNeeded();

	void Think();

	int32_t SamePortPatchup();

	uint8_t justinited;

	const char masterservs[A2S_MASTERSERVERCOUNT][30] = { "208.64.200.52", "208.64.200.39", "208.64.200.65", "46.165.194.16", "46.4.71.67", "46.165.194.14" };

	int32_t imasterservs[A2S_MASTERSERVERCOUNT] = { 0 };
	
	steady_clock::time_point lastrefreshtime;
	steady_clock::time_point lastrefreshtimemaster;
	steady_clock::time_point lastrefreshtime_a2sply;
	
	steady_clock::time_point a2sply_firstbuild;
	
	bool wearemaster;
	
	uint16_t recvport;
	int32_t recvsize;
	char recvip[30];

	char srca2sinfo[A2S_MAXDATASIZE];
	int32_t srca2sinfo_len;

	char srca2splayer[A2S_MAXDATASIZE];
	char srca2splayer_split[A2S_MAXDATASIZE];
	int32_t srca2splayers_len;
	
	char a2srecvdata[A2S_MAXDATASIZE];
	char a2ssenddata[A2S_MAXDATASIZE];
	long plychallenge;
	
	int8_t plyreducecnt = 3;
	int32_t lastsendlistupdate = -1;
	uint32_t curmasterserv;
	
	a2sinfo servera2s;
	a2sply servera2sply;
	
	ProxyServer *sameportproxy;
	WorkerData *worker;
	leynet_udp *udp;
	
	SourceSplitPacket splitpackets[30];
	int8_t splitpackets_i = 0xFF;
	int8_t requestwait = 0;
	
	uint32_t ournodeip = 0;
	std::vector <node> nodes;
	
};