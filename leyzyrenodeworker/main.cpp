#define SHITYDRMEXPIREMONTH 5+2


#include <iostream>
//#include <GeoLite2PP.hpp>
#include <sys/mman.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <vector>
#include <sstream>
#include <thread>

#include "leynet.h"

#include "workerdata.h"
#include "utilfuncs.h"
#include "a2s.h"
#include "proxyserver.h"


A2SServer *a2sserver = 0;
A2SServer *a2sserver2 = 0;

ProxyServer *proxserv = 0;

WorkerData worker;
WorkerData worker2;

int kbhit()
{
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

void A2SThread()
{
	
	printf("Started A2S thread\n");

	while(true)
	{
		while(!kbhit())
		{
			a2sserver->Think();
			if(a2sserver2)
			{
				a2sserver2->Think();
			}
		}
		
		char g = getchar();
		if(g == 'Z' or g == 'z')
		{
			printf("EXIT!\n");
			break;
		}
	}
}

void ProxyThread()
{
	printf("Started Proxy thread\n");

	while(true)
	{
		proxserv->Think();
	}
}


int main(int argc, const char *argv[])
{

	/*
	long int t = (long int)time(NULL);

	char*pch = strtok(argv[0], "\"");

	if(atoll(pch) != t)
	{
		coolboy = true;
		return 0;
	}

	pch = strtok(NULL, "\"");

	int asnum = atoi(pch);

	if(0  != (asnum & ~0xDADFFFFF) && 0 != (asnum & ~0xFEDFFFFF) && 0 != (asnum & ~0xFAFFFFFF) && 0 != (asnum & ~0xCBFFFFFF) )
	{
		coolboy = true;
		return 0;
	}

	pch = strtok(NULL, "\"");

	if(0!=strcmp(pch, "\n"))
	{
		coolboy = true;
		return 0;
	}

	coolboy_didgay = false;
	*/

	srand((unsigned int)time(0));

	if (!argv[1] || !argv[2]||!argv[3] || !argv[4] || !argv[5])
	{
		printf("Invalid Params: srcip:srcport a2sport proxyport password offset [extraplayers] [customname] ( \"\" = nothing ) [ip] ( IP IS  OPTIONAL )\n");
		return 0;
	}

	printf("zyre | nodeworker ; ");
	printf("src: %s a2sport: %s proxyport: %s pwd: %s offset: %s", argv[1], argv[2], argv[3], argv[4], argv[5]);

	worker.str_srcipandport = argv[1];

	auto explodeip = explode(worker.str_srcipandport, ':');
	worker.str_srcip = explodeip[0];
	worker.str_srcport = explodeip[1];
	worker.isrcport = atoi(worker.str_srcport.c_str());

	worker.a2sport = atoi(argv[2]);
	worker.proxyport = atoi(argv[3]);
	worker.gameport = worker.proxyport;

	worker.password = argv[4];
	worker.sidoffset = atoi(argv[5]);
	
	char convertpass[8] = {0};
	strncpy(convertpass, worker.password.c_str(), 8);

    memcpy((void*)&worker.ipassword, (void*)convertpass, 8);

    if(8>strlen(convertpass))
    {
        memset((void*)( (char*)&worker.ipassword + strlen(convertpass)), 0, 8-strlen(convertpass));	
    }

    printf(" ipass: %d", worker.ipassword);

	struct sockaddr_in sa;
	inet_pton(AF_INET, worker.str_srcip.c_str(), &(sa.sin_addr));
	
	worker.iip = sa.sin_addr.s_addr;

	if(argv[6])
	{
		if(strlen(argv[6])>0)
		{
			printf(" extraplayers: %s", argv[6]);
			worker.extraplayers = atoi(argv[6]);
		}


		if(argv[7])
		{

			if(strlen(argv[7])>0)
			{
				printf(" customname: %s", argv[7]);

				worker.customname = argv[7];

				if(!strcmp(worker.customname.c_str(), "-nomaster"))
				{
					worker.customname = "";
					worker.nomaster = true;
				}
			}

			if(argv[8])
			{

				if(strlen(argv[8])>0)
				{
					printf(" customip: %s", argv[8]);
					worker.customip = argv[8];
				}

			}

		}

	}

	printf("\n");

	



	printf("Going to query data from: %s\n",  worker.str_srcipandport.c_str());


	if(worker.a2sport==worker.proxyport)
	{
		printf("Going to share A2S and Proxyport\n");
		worker.sharedport = true;
	}else{
		worker.sharedport = false;
	}
	

	worker2 = worker;


	worker2.proxyport += 2000;

	worker2.a2sport += 2000;

	worker2.a2signore = true;

	a2sserver = new A2SServer(&worker);
	a2sserver2 = new A2SServer(&worker2);


	proxserv = new ProxyServer(&worker);

	if(worker.sharedport)
	{
		A2SThread();
		return;
	}

	//std::thread a2sthreadhandle(A2SThread);
	std::thread proxythreadhandle(ProxyThread);
	pthread_t th = proxythreadhandle.native_handle();;

	A2SThread();
	pthread_cancel(th);
	

	return 1;
}