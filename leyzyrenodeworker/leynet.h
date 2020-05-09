#ifndef LEYNET_H
#define LEYNET_H
#pragma once

#include <stdint.h>

class leynet
{
protected:
	bool customerrorhandling;
	bool nonblock;

	int32_t sock;
	uint16_t uport;

	char* HandleError();
public:

	inline void SetSock(int32_t ssock)
	{
		sock = ssock;
	}

	char* SetNonBlocking(bool block);
	char* SetIgnoreChecksum(bool ignoresum);
	
	//settings

	uint16_t GetPort();
	int32_t IPStrToInt(char* ip);

};

class leynet_udp : public leynet
{
private:

public:

	leynet_udp()
	{
		sock = 0;
		uport = 0;
		nonblock = 0;
		lastiip = 0;
	}

	~leynet_udp()
	{
		sock = 0;
		uport = 0;
		nonblock = 0;
		lastiip = 0;
	}

	//opening/closing/starting

	char *Start(bool customerr = false);

	char *OpenSocket(unsigned short port, const char*ip = 0);
	char *CloseSocket();



	//actions

	char *SendTo(int32_t numip, uint16_t port, const char *buffer, int32_t len);
	char *SendTo(const char*ip, uint16_t port, const char *buffer, int32_t len);
	char *Receive(int32_t*msgsize, uint16_t*port, char*ip, char*buffer, int32_t buffersize);

	int32_t lastiip;
};






class leynet_tcp : public leynet
{
private:
	char connectip[25];

public:
	typedef bool(*TCP_Finished)(uint32_t datalen, uint32_t curdatalen, char*buffer, char* curbuffer);

	uint32_t lenfin;
	int32_t timefin;
	int32_t timefin_s;
	int32_t lastiip;

	leynet_tcp()
	{
		lastiip = 0;
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;
		sock = 0;
		uport = 0;
		customerrorhandling = 0;
		nonblock = 0;

		for (int32_t i = 0; i < 25; i++)
			connectip[i] = 0;
		

	}

	~leynet_tcp()
	{
		lastiip = 0;
		lenfin = 0;
		timefin = 0;
		timefin_s = 0;
		sock = 0;
		uport = 0;
		customerrorhandling = 0;
		nonblock = 0;

		for (int32_t i = 0; i < 25; i++)
			connectip[i] = 0;
	}

	//opening/closing/starting

	char* Start(bool customerr = false);

	char *Bind(uint16_t port);
	char* Listen(char*ip, uint16_t&port, uint32_t*sock);
	char* OpenConnection(char*addr, uint16_t port);
	char* CloseConnection();

	//actions
	char* GetIP();
	char* HTTPParse(int32_t* msgsize, char* response);
	char* HTTPParseLength(int32_t* msgsize, char* response);
	char* HTTPGet(const char *resource);
	char* Send(const char *buffer, int32_t len);
	char* Receive(int32_t* msgsize, char*buffer, int32_t buffersize, TCP_Finished fin = 0);


	//Finish functions
	bool TLenFin(uint32_t datalen, uint32_t curdatalen, char*buffer, char* curbuffer);
	bool THTTPLenFin(uint32_t datalen, uint32_t curdatalen, char*buffer, char *curbuffer);

};

#endif