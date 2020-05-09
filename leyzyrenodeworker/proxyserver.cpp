
#include <stdio.h>
#include <string.h>
#include <chrono>
#include <unistd.h>

#include "proxyserver.h"
#include "workerdata.h"
#include "leybuf.h"

using std::chrono::steady_clock;

ProxyServer::~ProxyServer()
{
	if(udp)
	{
		udp->CloseSocket();
		delete udp;
		udp = 0;
	}

	if(worker)
	{
		delete worker;
		worker = 0;
	}

}

ProxyServer::ProxyServer(WorkerData *setworker)
{
	memset(proxyrecvdata, 0, PROXY_MAXDATASIZE);
	memset(proxysenddata, 0, PROXY_MAXDATASIZE);

	recvsize = 0;

	worker = new WorkerData;
	memcpy(worker, setworker, sizeof(WorkerData));

	memset(recvip, 0, 30);
}

void ProxyServer::WriteProxyHeader(leybuf *towrite, uint32_t numip, uint16_t port, uint8_t flags)
{
	towrite->WriteInt64(worker->ipassword);
	towrite->WriteInt32(numip);
	towrite->WriteInt16(port);
	towrite->WriteChar(flags);
}

void ProxyServer::OnReceiveSourceServerProxyData()
{
	leybuf readsourceserverdata(this->proxyrecvdata, this->recvsize);
	
	if(8 + 4 + 2 + 1> this->recvsize)
	{
		return;
	}

	int64_t pass = readsourceserverdata.ReadInt64();

	if(readsourceserverdata.IsOverflowed())
	{
		return;
	}

	if(pass != worker->ipassword)
	{
		#ifdef DEBUG
		printf("wrong password\n");
		#endif
		return;
	}

	int32_t numip = readsourceserverdata.ReadInt32();
	

	uint16_t port = (uint16_t)readsourceserverdata.ReadInt16();



	char flags = readsourceserverdata.ReadChar();
	
	if(readsourceserverdata.IsOverflowed())
	{
		return;
	}
	
	int32_t datasize = readsourceserverdata.GetNumBytesLeft();

	if(datasize>0)
	{
		char* datapos = (char*)( this->proxyrecvdata + 8 + 4 + 2 + 1 );
		udp->SendTo(numip, port, datapos, datasize);

	}else{
		udp->SendTo(numip, port, "", 0);
	}

}

void ProxyServer::OnReceiveNoData() // if we're blocking this'll be called way less
{

}

void ProxyServer::OnReceiveClientData()
{
	leybuf writedata(proxysenddata, PROXY_MAXDATASIZE);
	this->WriteProxyHeader(&writedata, udp->lastiip, this->recvport, 0);

	writedata.WriteBytes(this->proxyrecvdata, this->recvsize);

	udp->SendTo(worker->iip, worker->isrcport, writedata.GetData(), writedata.GetPos());

}


void ProxyServer::OnReceiveData()
{
	//printf("nice data bro: %s ___ %d\n", this->recvip, this->recvsize);


	if (worker->iip == udp->lastiip)
	{
		this->OnReceiveSourceServerProxyData();
	}else{
		this->OnReceiveClientData();
	}


}

void ProxyServer::Think()
{
	if(!worker)
	{
		return;
	}

	if(!udp)
	{
		printf("ProxyServer Initializing UDP\n");
		udp = new leynet_udp;
		udp->Start();

		if(worker->customip.length()>0)
		{
			udp->OpenSocket(worker->proxyport, worker->customip.c_str());
		}else{
			udp->OpenSocket(worker->proxyport);
		}

		udp->SetNonBlocking(true);
		udp->SetIgnoreChecksum(true);
		
		return;
	}

	char*worked = udp->Receive(&this->recvsize, &this->recvport, this->recvip, this->proxyrecvdata, PROXY_MAXDATASIZE);

	if (!this->recvsize || this->recvsize > 30000)
	{
		usleep(500);
		//this->OnReceiveNoData();
	}else{
		this->OnReceiveData();
	}
}