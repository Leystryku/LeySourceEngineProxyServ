#include "leynet.h"

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define WSAGetLastError() errno
#define closesocket close
#define SOCKADDR sockaddr
#endif

#include <string>
#include <string.h>

char* leynet::HandleError()
{

	int32_t wsaerr = WSAGetLastError();

	if (wsaerr == 10035 && !nonblock)
		return 0;

	if (wsaerr == 0)
		return (char*)"k";

	if (customerrorhandling)
	{
		char *errmsg = new char[255];
		sprintf(errmsg, "operation failed %i\n", wsaerr);
		return errmsg;
	}
#ifdef DEBUG
	printf("errmsg: operation failed %i\n", wsaerr);
#endif
	return (char*)"k";
}

int32_t leynet::IPStrToInt(char* ip)
{
	struct sockaddr_in sa;
	
	inet_pton(AF_INET, ip, &(sa.sin_addr));
	
	int iip = sa.sin_addr.s_addr;

	return iip;
}

char* leynet::SetNonBlocking(bool nonblocking)
{

	unsigned long mode = 0;

	if(nonblocking)
	{
		mode = 1;
	}else{
		mode = 0;
	}
	
	#ifdef _WIN32
	ioctlsocket(sock, FIONBIO, &mode);
	#else
	ioctl(sock, FIONBIO, &mode);
	#endif

	nonblock = nonblocking;

	return 0;
}


char *leynet::SetIgnoreChecksum(bool ignoresum)
{
	int yes = 1;
	if(!ignoresum)
	{
		yes = 0;
	}
	int rc = setsockopt(sock, SOL_SOCKET, SO_NO_CHECK, (void*)&yes, sizeof yes);
	
	if(rc != 0)
	{
		return HandleError();
	}
	
	return 0;
}


uint16_t leynet::GetPort()
{
	return uport;
}

//udp
//opening/closing/starting

char* leynet_udp::Start(bool customerr)
{
	if (customerr)
		customerrorhandling = true;

	#ifdef _WIN32
	uint16_t wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int32_t ret = WSAStartup(wVersionRequested, &wsaData);

	if (ret < 0)
		return HandleError();

	#endif
	
	return 0;
}

char* leynet_udp::OpenSocket(uint16_t port, const char* ip=0)
{

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	sockaddr_in add;
	add.sin_family = AF_INET;

	if(ip)
	{
		add.sin_addr.s_addr = inet_addr(ip);
	}else{
		add.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	add.sin_port = htons(port);

	int32_t ret = bind(sock, (SOCKADDR*)(&add), sizeof(add));

	if (ret < 0)
		return HandleError();

	uport = port;

	return 0;
}

char* leynet_udp::CloseSocket()
{
	if (sock)
		closesocket(sock);

	sock = 0;
	uport = 0;
	nonblock = 0;

	return 0;
}

//actions

char* leynet_udp::SendTo(int numip, uint16_t port, const char *buffer, int32_t len)
{

	if (!sock)
		return (char*)"k";

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_port = htons(port);
	add.sin_addr.s_addr = numip;

	
	int flags = 0;

	if(nonblock)
	{
		flags =  flags | MSG_DONTWAIT;
	}
	
	int32_t ret = sendto(sock, buffer, len, flags, (SOCKADDR *)&add, sizeof(add));


	if (ret < 0)
		return HandleError();

	return 0;

}

char* leynet_udp::SendTo(const char*ip, uint16_t port, const char *buffer, int32_t len)
{

	if (!sock)
		return (char*)"k";

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_port = htons(port);


	inet_pton(AF_INET, ip, &(add.sin_addr));

	int flags = 0;

	if(nonblock)
	{
		flags =  flags | MSG_DONTWAIT;
	}
	
	int32_t ret = sendto(sock, buffer, len, flags, (SOCKADDR *)&add, sizeof(add));


	if (ret < 0)
		return HandleError();

	return 0;

}

char *leynet_udp::Receive(int32_t*msgsize, uint16_t*port, char*ip, char*buffer, int32_t buffersize)
{
	if (!sock)
		return (char*)"k";

	if(msgsize)
	{	
		*msgsize = 0;
	}
		
	sockaddr_in from;

	#ifdef _WIN32
	int32_t size = sizeof(from);
	#else
	uint32_t size;
	size = sizeof(from);
	#endif

	int flags = 0;

	if(nonblock)
	{
		flags =  flags | MSG_DONTWAIT;
	}

	int32_t ret = recvfrom(sock, buffer, buffersize, flags, (SOCKADDR*)&from, &size);


	if (ret < 0)
		return HandleError();

	if(2 > ret)
	{
		lastiip = 0;
		
		if(ip)
		{
			ip[0] = 0;
		}
		
		buffer[0] = 0;
		return 0;
	}

	if (ret > buffersize)
		ret = buffersize - 1;

	buffer[ret + 1] = 0;
	lastiip = from.sin_addr.s_addr;
	
	if(ip)
	{
		inet_ntop(AF_INET, &from.sin_addr, ip, INET_ADDRSTRLEN);
	}
	

	if(msgsize)
	{
		*msgsize = ret;
	}
	if(port)
	{	
		*port = ntohs(from.sin_port);
	}
	
	return 0;
}


//tcp
//opening/closing/starting

char* leynet_tcp::Start(bool customerr)
{
	if (customerr)
		customerrorhandling = true;

	#ifdef _WIN32
	uint16_t wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	int32_t ret = WSAStartup(wVersionRequested, &wsaData);

	if (ret < 0)
		return HandleError();

	#endif

	return 0;
}

char* leynet_tcp::Bind(uint16_t bport)
{
	sock = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_addr.s_addr = htonl(INADDR_ANY);
	add.sin_port = htons(bport);

	int32_t ret = bind(sock, (SOCKADDR*)&add, sizeof(add));


	if (ret < 0)
	{
		closesocket(sock);
		return HandleError();
	}

	listen(sock, 3);

	return 0;
}

char* leynet_tcp::Listen(char*ip, uint16_t&lport, uint32_t*ssock)
{

	sockaddr_in client;

	

	#ifdef _WIN32
	int32_t len = sizeof(sockaddr_in);
	#else
	uint32_t len;
	len = sizeof(sockaddr_in);
	#endif

	uint32_t accepted = (uint32_t)accept(sock, (sockaddr*)&client, &len);

	if (accepted<0)
	{

		return 0;
	}

	*ssock = accepted;

	inet_ntop(AF_INET, &client.sin_addr, ip, INET_ADDRSTRLEN);

	lastiip = client.sin_addr.s_addr;
	
	if (lport != 0)
		lport = ntohs(client.sin_port);

	return 0;
}

char* leynet_tcp::OpenConnection(char*addr, uint16_t port)
{

	addrinfo *hints = new addrinfo;
	memset(hints, 0, sizeof(*hints));
	hints->ai_family = AF_INET;
	hints->ai_protocol = IPPROTO_TCP;
	hints->ai_socktype = SOCK_STREAM;

	addrinfo* addrinfo = 0;

	int32_t ret = getaddrinfo(addr, NULL, hints, &addrinfo);

	if (ret < 0 || !addrinfo)
	{
		free(hints);
		return HandleError();
	}

	free(hints);

	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	uport = port;

	sockaddr_in add;
	add.sin_family = AF_INET;
	add.sin_port = htons(port);
	add.sin_addr = ((struct sockaddr_in*)addrinfo->ai_addr)->sin_addr;

	freeaddrinfo(addrinfo);

	ret = connect(sock, (SOCKADDR*)(&add), sizeof(add));

	if (ret<0)
	{
		closesocket(sock);
		sock = 0;
		return HandleError();
	}

	strcpy(connectip, addr);

	return 0;
}


char* leynet_tcp::CloseConnection()
{
	if (sock)
		closesocket(sock);

	sock = 0;
	uport = 0;
	nonblock = 0;

	for (int32_t i = 0; i < 25; i++)
		connectip[i] = 0;

	return 0;
}

//actions

char *leynet_tcp::HTTPParseLength(int32_t*msgsize, char*msg)
{
	int32_t size = *msgsize;

	int32_t newsize = 0;

	char *foundlength = strstr(msg, "Content-Length:");

	if (foundlength)
	{
		printf("found length!\n");

		char *start = foundlength + 16;

		int32_t endlen = 1;

		for (endlen; endlen < 15; endlen++)
		{
			if (start[endlen] == '\r'&&start[endlen + 1] == '\n')
				break;
		}

		char contentlength[50];
		memcpy(contentlength, start, endlen);
		contentlength[endlen] = 0;

		newsize += strtol(contentlength, 0, 10);
	}

	for (int32_t i = 0; i < size; i++)
	{
		/*
		if (msg[i] == '\r'&&msg[i + 1] == '\n'&&msg[i + 2] == '\r'&&msg[i + 3] == '\n')
		{
		i = i + 4;
		continue;
		}*/

		if (msg[i] != '\r' || msg[i + 1] != '\n')
		{
			continue;
		}

		if (!isalnum(msg[i + 2]))
			continue;


		bool bfound = false;

		int32_t a = 0;
		for (a = 2; a < 12; a++)
		{
			if (msg[i + a] == '\r'&&msg[i + a + 1] == '\n')
			{
				bfound = true;
				break;
			}

		}

		if (!bfound)
			continue;



		char numstr[20];
		memset(numstr, 0, 20);
		memcpy(numstr, msg + i + 2, a - 2);


		int32_t skipper = strtol(numstr, 0, 16);



		newsize += skipper;
	}

	*msgsize = newsize;

	return 0;
}

char *leynet_tcp::HTTPParse(int32_t* msgsize, char* msg)
{

	int32_t size = *msgsize;

	if (!size)
		return 0;

	char *header = new char[16000];

	for (int32_t i = 0; i < size; i++)
	{
		if (msg[i] == '\r'&&msg[i + 1] == '\n'&&msg[i + 2] == '\r'&&msg[i + 3] == '\n')
		{
			memcpy(header, msg, i + 2);//save header
			header[i + 3] = 0;

			memcpy(msg, msg + i + 2, size - i);//remove header from raw msg
			size = size - i;
			break;
		}
	}

	if (strlen(header) == 0)
	{
		return 0;
	}


	char *foundlength = strstr(header, "Content-Length:");

	int32_t newsize = 0;

	if (foundlength)
	{
		printf("found length!\n");

		char *start = foundlength + 16;

		int32_t endlen = 1;

		for (endlen; endlen < 15; endlen++)
		{
			if (start[endlen] == '\r'&&start[endlen + 1] == '\n')
				break;
		}

		char contentlength[50];
		memcpy(contentlength, start, endlen);
		contentlength[endlen] = 0;

		newsize = strtol(contentlength, 0, 10);
	}


	printf("chunked..\n");


	for (int32_t i = 0; i < size; i++)
	{
		if (msg[i] != '\r' || msg[i + 1] != '\n')
			continue;

		bool bfound = false;

		int32_t a = 0;
		for (a = 1; a < 12; a++)
		{
			if (msg[i + a] == '\r'&&msg[i + a + 1] == '\n')
			{
				bfound = true;
				break;
			}

			if (!isalnum(msg[i + a]))
				break;

		}

		if (!bfound)
			continue;

		char numstr[20] = { 0 };

		memcpy(numstr, msg + i + 2, a - 2);

		int32_t skipper = strtol(numstr, 0, 16);

		memcpy(msg + i, msg + i + a + 1, size - i);

		if (!foundlength)
		{
			newsize += skipper;
		}
	}


	if (msg[0] == '\r'&&msg[1] == '\n')
	{
		memcpy(msg, msg + 2, newsize);
	}

	delete[] header;

	size = newsize;
	*msgsize = size;
	return 0;
}

char* leynet_tcp::GetIP()
{
	return connectip;
}

char* leynet_tcp::HTTPGet(const char *resource)
{
	char request[800];
	sprintf(request, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: Keep-Alive\r\n\r\n", resource, connectip);

	char *ret = Send(request, strlen(request));

	if (ret)
		return ret;

	return 0;
}

char* leynet_tcp::Send(const char *buffer, int32_t len)
{
	if (!sock)
		return (char*)"k";

	int32_t ret = send(sock, buffer, len, 0);

	if (ret < 0)
		return HandleError();

	return 0;

}

char *leynet_tcp::Receive(int32_t* msgsize, char*buffer, int32_t buffersize, TCP_Finished fin)
{

	uint32_t totaldatalen = 0;

	int32_t curbuffer_size = 0xFFFFFFF;
	if (curbuffer_size > buffersize)
		curbuffer_size = buffersize - 3;

	char*curbuffer = new char[curbuffer_size];

	while (true)
	{

		if (buffer && totaldatalen > (uint32_t)buffersize)
			break;

		memset(curbuffer, 0, curbuffer_size);

		int32_t curdatalen = recv(sock, curbuffer, curbuffer_size, 0);
		if (curdatalen > 0)
		{

			if (buffer)
				memcpy(buffer + totaldatalen, curbuffer, curdatalen);

			totaldatalen += curdatalen;
		}

		if (fin)
		{

			bool is_fin = false;

			if (curdatalen <= 0)
			{
				is_fin = fin(totaldatalen, 0, buffer, curbuffer);
			}
			else {
				is_fin = fin(totaldatalen, curdatalen, buffer, curbuffer);
			}

			if (is_fin)
			{

				break;
			}
		}

		if (!fin)
			break;

	}


	delete[] curbuffer;

	if (buffer)
		buffer[totaldatalen + 1] = 0;

	*msgsize = *msgsize + totaldatalen;


	return 0;
}

bool leynet_tcp::TLenFin(uint32_t datalen, uint32_t curdatalen, char*buffer, char*curbuffer)
{

	if (lenfin&&datalen >= lenfin)
	{
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;

		return true;
	}

	if (timefin)
	{
	    time_t t = time(0);
	    struct tm *now = localtime(&t);


		if (!timefin_s)
		{
			timefin_s = now->tm_sec;
		}

		if (abs(timefin_s - now->tm_sec) >= timefin)
		{
			lenfin = 0;
			timefin = 0;
			timefin_s = 0;
			return true;
		}

		return false;
	}

	return true;
}

bool leynet_tcp::THTTPLenFin(uint32_t datalen, uint32_t curdatalen, char*buffer, char*curbuffer)
{

	int32_t penis = curdatalen;

	HTTPParseLength(&penis, curbuffer);

	lenfin += penis;

	printf("DATALEN: %i LENFIN: %i\n", datalen, lenfin);

	if (lenfin&&datalen >= lenfin)
	{
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;

		return true;
	}

	if (timefin)
	{
	    time_t t = time(0);
	    struct tm *now = localtime(&t);


		if (!timefin_s)
		{
			timefin_s = now->tm_sec;
		}

		if (abs(timefin_s - now->tm_sec) >= timefin)
		{
			lenfin = 0;
			timefin = 0;
			timefin_s = 0;
			return true;
		}

		return false;
	}



	return true;
}