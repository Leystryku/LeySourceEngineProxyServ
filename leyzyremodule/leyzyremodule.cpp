#define MAXIPS 100000


#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <stdio.h>
#include <cstdio>
#include <chrono>
#include <stdlib.h>
#include <unistd.h>
#include <curl/curl.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

std::string weburl = "https://zyre.club/zyre/";

ssize_t (*old_recvfrom)(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t (*old_sendto)(int sockfd, void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);


using std::chrono::steady_clock;

struct udpconnection
{
	unsigned long real_ip;
	unsigned short real_port;

	unsigned long proxy_ip;
	unsigned short proxy_port;
	bool is_real;

	sockaddr_in sendaddr;

	std::string statusstring;

	steady_clock::time_point lastreply;

};

std::vector <udpconnection*> Connections;


char cproxypassword[9] = {0};
int64_t proxypassword = 0;

char *FindString(char *p, const char *string, unsigned int len)
{

	for (unsigned int i = 0; i < len ; i++)
	{
		if (!memcmp((p + i), string, strlen(string) +1))
		{
			return (p + i);
		}
	
	}

	return 0;
}

char *FindAddrUse(char *p, void*addr, unsigned int len)
{
	for (unsigned int i = 0; i < len; i++)
	{

		if (memcmp(&p[i], &addr, 4))
			continue;

		return (char*)(p + i + 4);

	}
	
	return 0;
}


uint32_t  alt_getpid(const char * name)
{

	char cmd[500] = {0};
	char buf[500] = {0};

	snprintf(buf, 512, "pidof -s %s", name);
	FILE* maps = popen(buf, "r");

	if(maps)
	{
		if(fgets(buf, 512, maps));
	}

	uint32_t pid = strtoul(buf, NULL, 10);

	pclose( maps );
	return pid;
}

uint32_t getmodule(const char * modname, uint32_t pid)
{

	char cmd[500] = {0};
	char buf[500] = {0};

	snprintf(cmd, 256, "grep \"%s\" /proc/%i/maps | head -n 1 | cut -d \"-\" -f1", modname, pid);
	FILE* maps = popen(cmd, "r");

	uint32_t result = 0;
	if(maps)
	{
		if(fscanf(maps, "%08lx", &result));
	}

	pclose(maps);
	return result;
}



bool hookedsteam = false;

char statusip[0xFF] = {0};

int latestversion = 0;
int ourversion = 0;
int ourport = 0;

struct WebMemoryStruct
{
	char *memory;
	size_t size;
};


WebMemoryStruct webdata = {0};

size_t WebWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	WebMemoryStruct *mem = (WebMemoryStruct *)userp;

	mem->memory = realloc(mem->memory, mem->size + realsize + 1);
	if(mem->memory == NULL) {
		/* out of memory! */
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

int CleanWebData()
{
	if(!webdata.memory)
		return;

	memset(webdata.memory, 0, webdata.size);
	free(webdata.memory);
	webdata.memory = 0;
	webdata.size = 0;

	memset(&webdata, 0, sizeof(webdata));

	return 0;
}

int FetchWebData(const char*url)
{

	CleanWebData();

	if(!url)
	{
		printf("*url points to nothing!\n");
		return 1;
	}

	webdata.memory = malloc(1);
	memset(webdata.memory, 0, 1);

	webdata.size = 0;

	CURL *curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WebWriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&webdata);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");


	CURLcode res = curl_easy_perform(curl_handle);

	if(res != CURLE_OK)
	{
		printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		CleanWebData();

		return 1;
	}

	if(6>webdata.size || strncmp((char*)webdata.memory, "suc: ", 5) && strncmp((char*)webdata.memory, "err: ", 5))
	{
		printf("Reply didn't contain suc or err! [%s]", url);
		printf("%s\n", ((char*)webdata.memory));

		CleanWebData();
		return 1;
	}
	
	memmove(webdata.memory, (char*)webdata.memory+5, webdata.size-5);
	char*cmem = (char*)webdata.memory;
	cmem[webdata.size - 5]= 0;

	curl_easy_cleanup(curl_handle);

	return 0;
}

inline bool FileWrite(const char*filename, char* content, size_t contentlength)
{
	FILE *fhandle = fopen(filename, "wb");
	
	if(!fhandle)
	{
		return false;
	}

	fwrite(content, sizeof(char), contentlength, fhandle);

	fclose(fhandle);

}

inline char* FileRead(const char*filename, int* filesize)
{
	FILE *fhandle = fopen(filename, "rb");
	
	if(!fhandle)
	{
		return 0;
	}

	fseek(fhandle, 0, SEEK_END);
	int contentlength = (int)ftell(fhandle);

	if(filesize)
	{
		*filesize = contentlength;
	}

	fseek(fhandle,0, SEEK_SET);

	if(contentlength > 0xFFFFFFF)
	{
		printf("File read failed, file too big!\n");
		fclose(fhandle);
		return 0;
	}

	char*content = new char[contentlength+1];
	content[contentlength] = 0;
	content[contentlength+1] = 0;

	fread(content, sizeof(char), contentlength, fhandle);
	fclose(fhandle);

	return content;
}


inline bool NeedsUpdate()
{
	char cversion[8] = {0};
	sprintf(cversion, "%d", ourversion);
	char cport[8] = {0};
	sprintf(cport, "%d", ourport);

	std::string updateurl = weburl + "autoupdate.php?checkver=1&currentversion=" + cversion;
	updateurl = updateurl + "&gsport=";
	updateurl = updateurl + cport;

	if(FetchWebData(updateurl.c_str()))
	{
		printf("Fetching update failed!\n");
		return;
	}


	char*morerecent = (char*)webdata.memory;

	if(!strcmp(morerecent, "up to date"))
	{
		printf("We're up to date!\n");
		return false;
	}

	latestversion = atoi(morerecent);

	if(strlen(morerecent)==0 || ourversion>latestversion)
	{
		printf("We're up to date!\n");
		return false;
	}

	printf("LATEST: %d\n", latestversion);


	return true;
}

inline bool FetchKey()
{
	char cversion[8] = {0};
	sprintf(cversion, "%d", ourversion);
	char cport[8] = {0};
	sprintf(cport, "%d", ourport);

	std::string keyurl = weburl + "serverdata.php?gsport=";
	keyurl = keyurl + cport;

	printf("ZyreCL | Grabbing our key!\n");

	if(FetchWebData(keyurl.c_str()))
	{
		printf("ZyreCL: Key fetching failed, retry!\n");
		usleep(1000*1000);
		return FetchKey();
	}

	printf("ZyreCL | Grabbed our key %s\n", (char*)webdata.memory);

	strncpy(cproxypassword, (char*)webdata.memory,8);
    memcpy((void*)&proxypassword, (void*)cproxypassword, 8);

    if(8>cproxypassword)
    {
        memset((void*)( (char*)&proxypassword + strlen(cproxypassword)), 0, 8-strlen(cproxypassword));	
    }


	printf("ZyreCL | Saved leyzyre_changelog.txt\n");

	return true;
}

inline bool DownloadChangelog()
{
	char cversion[8] = {0};
	sprintf(cversion, "%d", ourversion);
	char cport[8] = {0};
	sprintf(cport, "%d", ourport);

	std::string updateurl = weburl + "autoupdate.php?changelog=1&currentversion=" + cversion;
	updateurl = updateurl + "&gsport=";
	updateurl = updateurl + cport;

	if(FetchWebData(updateurl.c_str()))
	{
		return false;
	}

	printf("ZyreCL | Downloading new changelog...\n");

	FileWrite("leyzyre_changelog.txt", webdata.memory, webdata.size);

	printf("ZyreCL | Saved leyzyre_changelog.txt\n");

	return true;
}

inline bool DownloadUpdate()
{
	char cversion[8] = {0};
	sprintf(cversion, "%d", ourversion);
	char cport[8] = {0};
	sprintf(cport, "%d", ourport);

	std::string updateurl = weburl + "autoupdate.php?currentversion=" + cversion;
	updateurl = updateurl + "&gsport=";
	updateurl = updateurl + cport;

	printf("ZyreCL | Downloading new worker...\n");

	if(FetchWebData(updateurl.c_str()))
	{
		return false;
	}

	FileWrite("leyzyrecl_new.so", webdata.memory, webdata.size);
	printf("ZyreCL | Saved leyzyrecl_new.so\n");

	ourversion = latestversion;

	memset(cversion, 0, 8);
	sprintf(cversion, "%d", ourversion);

	FileWrite("leyzyre_version.txt", cversion, strlen(cversion));

	printf("ZyreCL | Saved leyzyre_version.txt\n");

	printf("ZyreCL | Replacing old zyre version...\n");

	if(rename("leyzyrecl.so", "leyzyrecl_old.so"))
	{
		printf("ZyreCL | Error renaming old module!\n");
		return false;
	}

	remove("leyzyrecl_old.so");

	if(rename("leyzyrecl_new.so", "leyzyrecl.so"))
	{
		printf("ZyreCL | Error renaming new module!\n");
		return false;
	}

	printf("ZyreCL | Installed new module\n");
	printf("ZyreCL | It'll be loaded on next start!\n");

	return true;
}

inline bool CheckForUpdates()
{
	char*readourversion = FileRead("leyzyre_version.txt", 0);

	if(readourversion)
	{
		ourversion = atoi(readourversion);

		printf("ZyreCL | Revision: %d\n", ourversion);
	}else{
		printf("ZyreCL | No version saved.\n");
	}

	printf("=== Checking for updates...\n");

	if(NeedsUpdate())
	{
		printf("ZyreCL | Update required! Downloading...\n");
		DownloadChangelog();
		DownloadUpdate();

		return true;
	}

	return false;
}

inline void CheckSteamHooks()
{
	if(hookedsteam)
		return;

	Connections.clear();

	uint32_t ourpid = (uint32_t)getpid();
	
	if(!ourpid)
	{
		ourpid = alt_getpid("srcds_linux");
	}

	if(!ourpid)
	{
		return;
	}

	hookedsteam = true;
	printf("ZyreCL | Modifying the games code...\n");

	char* engineso = (char*)getmodule("engine.so", (uint32_t)ourpid);

	if(!engineso)
		engineso = (char*)getmodule("engine_srv.so", (uint32_t)ourpid);

	if(!engineso)
	{
		printf("ZyreCL |  Couldn't find engine\n");
		return;
	}
	printf("ZyreCL |  Doing the hookerings\n");

	//VERSION
	char* Version_String = (char*)FindString((char*)engineso, "version : %s/%d %d %s%s%s\n", 0x94C000);//actual func name

	if(!Version_String)
	{
		printf("ZyreCL | Couldn't find status::Version_String.\n");
		return 0;
	}

	printf("ZyreCL | Found status string\n");


	char* Version_StringRef = FindAddrUse((char*)engineso, Version_String, 0x94C000);

	if(!Version_StringRef)
	{
		printf("ZyreCL | Couldn't find any usage of the status string.\n");
		return 0;
	}


	printf("ZyreCL | Found status string ref\n");


	Version_StringRef = Version_StringRef - 4;

	int result = mprotect((int)Version_StringRef & ~(4095), 4096*2, PROT_WRITE | PROT_READ);

	if(result)
	{
		printf("ZyreCL | first mprotect failed: %d\n", result);
		return;
	}

	char* ourversionstring = new char[256];
	memset(ourversionstring, 0, 256);
	strcpy(ourversionstring, "version : %s/%d %d secure\n\x00 %s %s\n");

	int* Version_StringAddr = (int*)Version_StringRef;
	*Version_StringAddr = ourversionstring;

	result = mprotect((int)Version_StringRef & ~(4095), 4096*2, PROT_EXEC | PROT_READ);

	if(result)
	{
		printf("ZyreCL | second mprotect failed: %d\n", result);
		return;
	}


	//udp/ip  : %s:%i%s
	char* IP_String = (char*)FindString((char*)engineso, "udp/ip  : %s:%i%s\n", 0x94C000);//actual func name

	if(!IP_String)
	{
		printf("ZyreCL | Couldn't find status::IP_String.\n");
		return 0;
	}

	printf("ZyreCL | Found status string 2\n");


	char* IP_StringRef = FindAddrUse((char*)engineso, IP_String, 0x94C000);

	if(!IP_StringRef)
	{
		printf("ZyreCL | Couldn't find any usage of the status string.\n");
		return 0;
	}


	printf("ZyreCL | Found status string ref 2\n");


	IP_StringRef = IP_StringRef - 4;

	result = mprotect((int)IP_StringRef & ~(4095), 4096*2, PROT_WRITE | PROT_READ);

	if(result)
	{
		printf("ZyreCL | first mprotect failed: %d\n", result);
		return;
	}

	memset(statusip, 0, sizeof(statusip));
	strcpy(statusip, "udp/ip  : 127.0.0.1:27015\n\x00%s%i%s");

	int* IP_StringAddr = (int*)IP_StringRef;
	*IP_StringAddr = statusip;

	result = mprotect((int)IP_StringRef & ~(4095), 4096*2, PROT_EXEC | PROT_READ);

	if(result)
	{
		printf("ZyreCL | second mprotect failed: %d\n", result);
		return;
	}

	//actual important auth work
	char* Disconnect_String = (char*)FindString((char*)engineso, "Disconnect by server.\n", 0x94C000);//actual func name

	if(!Disconnect_String)
	{
		printf("ZyreCL | Couldn't find 'Disconnect by server.' ");
		return 0;
	}



	printf("ZyreCL | Found CNetChan::Clear string\n");

	char* Disconnect_StringRef = FindAddrUse((char*)engineso, Disconnect_String, 0x94C000);

	if(!Disconnect_StringRef)
	{
		printf("ZyreCL | Couldn't find any usage of the disconnect string.");
		return 0;
	}

	printf("ZyreCL | Found string usage\n");

	char* fnd_end = 0;

	for (int i =0;i<0x200;i++)
	{
		char curop = *(char*)(Disconnect_StringRef + i);
		char nextop = *(char*)(Disconnect_StringRef + i + 1);


		if(curop != (char)0x5D || nextop != (char)0xC3 )
			continue;

		fnd_end = (char*)(Disconnect_StringRef + i);
		break;
	}

	if(!fnd_end)
	{
		printf("ZyreCL | Couldn't find CNetChan::Clear end!\n");
		return;
	}

	char* fnd_mov = 0;

	bool firstmove = true;

	for (int i =0;i<0x200;i++)
	{
		char curop = *(char*)(fnd_end - i - 1);
		char nextop = *(char*)(fnd_end - i);

		if(curop != (char)0xC6 || nextop != (char)0x87)
			continue;

		if(firstmove)
		{
			firstmove = false;
			continue;
		}

		fnd_mov = (char*)(fnd_end - i - 1);
		break;
	}

	if(!fnd_mov)
	{
		printf("ZyreCL | Couldn't find m_bSendServerInfo=false\n");
		return;
	}

	char* setnumber = (char*)(fnd_mov + 6);

	result = mprotect((int)setnumber & ~(4095), 4096*2, PROT_WRITE | PROT_READ);

	if(result)
	{
		printf("ZyreCL | first mprotect failed: %d\n", result);
		return;
	}

	if(*setnumber != 1)
	{
		*setnumber = 1;
	}
	

	result = mprotect((int)setnumber & ~(4095), 4096*2, PROT_EXEC | PROT_READ);

	if(result)
	{
		printf("ZyreCL | second mprotect failed: %d\n", result);
		return;
	}

	printf("ZyreCL | Done Initializing!\n");


}
inline int GetConnection( unsigned long ip, unsigned short port )
{


	for(auto it = Connections.begin(); it != Connections.end(); ++it)
	{
		udpconnection *connection = *it;

		if(connection->real_ip != ip || connection->real_port != port)
			continue;

		return connection;
	}

	return 0;

}

#define ExpireAfterSeconds 10


int nextcheck = 0;

inline void ExpireConnections()
{
	if(!nextcheck)
	{
		nextcheck = 10;
	}else{
		nextcheck = nextcheck -1;
		return;
	}

	steady_clock::time_point curtime = steady_clock::now();

	std::vector<udpconnection*> toremove;
	toremove.clear();

	for(auto it = Connections.begin(); it != Connections.end(); ++it)
	{
		udpconnection *connection = *it;

		if(!connection)
			break;

		steady_clock::time_point lastreply = connection->lastreply;

  		auto chrono_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastreply);
		auto mseconds = chrono_milliseconds.count();

		if(1000*ExpireAfterSeconds>mseconds)
			continue;

		toremove.push_back(connection);

	}

	for(auto it = toremove.begin(); it != toremove.end(); ++it)
	{

		udpconnection *connection = *it;

		if(!connection)
			break;

		Connections.erase(std::remove(Connections.begin(), Connections.end(), connection));

		delete connection;
	}

}


inline bool NewConnection(unsigned long real_ip, unsigned short real_port, unsigned long proxy_ip, unsigned short proxy_port, bool is_real)
{
	
	ExpireConnections();

	if(GetConnection(real_ip, real_port))
		return false;

	int fndslot = Connections.size();

	if(Connections.size() >= MAXIPS - 3)
	{
		ExpireConnections();
		return false;
	}


	udpconnection *newconnection = new udpconnection;
	newconnection->real_ip = real_ip;
	newconnection->real_port = real_port;
	newconnection->proxy_ip = proxy_ip;
	newconnection->proxy_port = proxy_port;
	newconnection->is_real = is_real;
	newconnection->lastreply = steady_clock::now();
	newconnection->statusstring = "";

	newconnection->sendaddr.sin_family = AF_INET;
	newconnection->sendaddr.sin_addr.s_addr = proxy_ip;
	newconnection->sendaddr.sin_port = htons(proxy_port);


	Connections.push_back(newconnection);

	//printf("new connection: %d\n", fndslot);

	return true;
}


size_t proxypkgheader = 8+4+2+1;

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, sockaddr *src_addr, socklen_t *addrlen)
{	
	if (!old_recvfrom)
	{
		old_recvfrom = dlsym(RTLD_NEXT,"recvfrom");
	}
	
	int msgsize = old_recvfrom(sockfd, buf, len, flags | MSG_DONTWAIT, src_addr, addrlen);


	if(1>msgsize)
	{
		usleep(200);
	}
	
	if (!sockfd||!buf)
	{
		return msgsize;
	}

			

	if(!ourport)
	{
		struct sockaddr_in oursin;

		socklen_t ourlen = sizeof(oursin);
		if (getsockname(sockfd, (struct sockaddr *)&oursin, &ourlen) == -1)
		{
			printf("Error with getsockname, couldn't acquire port!\n");
		}else{

			int thisport = (int)ntohs(oursin.sin_port);

			if(thisport > 27010 && 27050 > thisport)
			{
				printf("ZyreCL | Found port: %d\n", thisport);
				ourport = thisport;

				int yes = 1;
				setsockopt(sockfd, SOL_SOCKET, SO_NO_CHECK, (void*)&yes, sizeof yes);
				ioctl(sockfd, FIONBIO, &yes);
				
				curl_global_init(CURL_GLOBAL_ALL);

				CheckForUpdates();
				FetchKey();

			}

		}

	}
	
	if(msgsize > 3 && (*(unsigned int*)buf == 0xFFFFFFFF || *(unsigned int*)buf == 825250646)) // 825250646 = VS01
	{
		CheckSteamHooks();
		memset(buf, 0, msgsize);
		return 0;
	}
	if (proxypkgheader > msgsize)
	{
		CheckSteamHooks();
		return msgsize;
	}

	

	int64_t receivedpassword = *(int64_t*)buf;

	if(receivedpassword != proxypassword)
	{
		return msgsize; // no exploiting here
	}

	sockaddr_in *insrc_addr = (sockaddr_in*)src_addr;

	unsigned long proxy_ip = insrc_addr->sin_addr.s_addr;
	unsigned short proxy_port = ntohs(insrc_addr->sin_port);

	
	unsigned long real_ip = *(unsigned long*)(buf + 8);
	unsigned short real_port = *(unsigned short*)(buf + 8 + 4);
	
	char msgflags = *(char*)(buf + 8 + 4 + 2);

	bool is_real = false;

	if(real_ip == (unsigned long)-1 && real_port == (unsigned short)-1)
	{
		real_ip = proxy_ip;
		real_port = proxy_port;
		is_real = true;
	}

	udpconnection *connection = GetConnection(real_ip, real_port);

	if(!connection)
	{
		NewConnection(real_ip, real_port, proxy_ip, proxy_port, is_real);
	}else{
		
		connection->lastreply = steady_clock::now();

		if(connection->proxy_ip != proxy_ip || connection->proxy_port != proxy_port) // switched nodes
		{
			Connections.erase(std::remove(Connections.begin(), Connections.end(), connection));
			delete connection;
			
			NewConnection(real_ip, real_port, proxy_ip, proxy_port, is_real);
			connection = GetConnection(real_ip, real_port);
		}

		if(!connection->is_real)
		{
			if(connection->statusstring.length()==0)
			{
				char cip[32] = {0};
				inet_ntop(AF_INET, &insrc_addr->sin_addr, cip, INET_ADDRSTRLEN);

				char cport[32] = {0};
				sprintf(cport, ":%d", (int)proxy_port);

				connection->statusstring = connection->statusstring + "udp/ip  : ";
				connection->statusstring = connection->statusstring + cip;

				connection->statusstring = connection->statusstring + cport;
				connection->statusstring = connection->statusstring + "\n\x00%s%i%s";


			}

			memset(statusip, 0, sizeof(statusip));
			strcpy(statusip, connection->statusstring.c_str());
		}
	}




	//printf("receiving proxied traffic\n");
	//printf("proxy_ip: %d:%d == real_ip: %d==%d\n", proxy_ip, (int)proxy_port, real_ip, (int)real_port);



	memmove((char*)buf, (char*)(buf + proxypkgheader), msgsize - proxypkgheader);
	memset((char*)(buf + msgsize - proxypkgheader), 0, proxypkgheader);

	insrc_addr->sin_family = AF_INET;
	insrc_addr->sin_addr.s_addr = (unsigned long)real_ip;
	insrc_addr->sin_port = htons((unsigned short)real_port);

	return msgsize - proxypkgheader;
}

char* sendwritebuf = 0;

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)
{
	if (!old_sendto)
		old_sendto = dlsym(RTLD_NEXT,"sendto");

	if(!dest_addr)
	{
		return old_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	}


	sockaddr_in *origaddr = (sockaddr_in*)dest_addr;

	unsigned long send_ip = origaddr->sin_addr.s_addr;
	unsigned short send_port = (unsigned short)ntohs(origaddr->sin_port);


	udpconnection *connection = GetConnection(send_ip, send_port);

	if(!connection)
	{
		return old_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	}
	
	if(connection->is_real)
	{
		send_ip = -1;
		send_port = -1;
		
		steady_clock::time_point curtime = steady_clock::now();

		auto chrono_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastfilewrite);
		auto mseconds = chrono_milliseconds.count();
		if(mseconds > 5000)
		{
			
			FILE *fhandle = fopen("crashtext.txt", "wb");
			
			fclose(fhandle);
			
			lastfilewrite = curtime;
		}
	}
	
	//unsigned long proxy_ip = connection->proxy_ip;
	//unsigned short proxy_port = connection->proxy_port;



	//printf("sending proxied traffic\n");
	//printf("proxy_ip: %d:%d == send_ip: %d==%d\n", proxy_ip, (int)proxy_port, send_ip, (int)send_port);

	
	if(!sendwritebuf)
	{
		sendwritebuf = new char[0xFFFF];
	}

	*(int64_t*)sendwritebuf = proxypassword;
	*(unsigned long*)(sendwritebuf + 8) = send_ip;
	*(unsigned short*)(sendwritebuf + 8 + 4) = send_port;
	*(char*)(sendwritebuf + 8 + 4 + 2) = 0;

	memcpy(sendwritebuf + proxypkgheader, buf, len);


	return old_sendto(sockfd, sendwritebuf, len + proxypkgheader, flags | MSG_DONTWAIT, (sockaddr*)&connection->sendaddr, sizeof(connection->sendaddr));; 
}
