#pragma once
#define PROXY_MAXDATASIZE 30000
#define PROXY_CLIENTTIMEOUT 60000 * 3// clients time out after these many milliseconds of no contact

#define PROXY_MAXPATHSIZE 256 // max size for folders

#include <chrono>
#include <stdio.h>
#include <string.h>

#include <stdint.h>

#include "leynet.h"

using std::chrono::steady_clock;

class WorkerData;
class leybuf;

class ProxyServer
{
public:
	ProxyServer(WorkerData *setworker);
	~ProxyServer();

	void WriteProxyHeader(leybuf *towrite, uint32_t numip, uint16_t port, uint8_t flags);

	void OnReceiveNoData();
	void OnReceiveData();

	void OnReceiveSourceServerProxyData();

	void OnReceiveClientData();

	void Think();

	steady_clock::time_point lastrefreshtime;

	uint16_t recvport;
	int32_t recvsize;
	char recvip[30];


	char proxyrecvdata[PROXY_MAXDATASIZE];
	char proxysenddata[PROXY_MAXDATASIZE];

	WorkerData *worker;
	leynet_udp *udp;

};