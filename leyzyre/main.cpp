#define WEBCHECKFREQUENCY 60

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <algorithm>

#include <curl/curl.h>

#include <sys/types.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

std::vector<std::string> explode(std::string const & s, char delim)
{
    std::vector<std::string> result;
    std::istringstream iss(s);

    for (std::string token; std::getline(iss, token, delim); )
    {
        result.push_back(std::move(token));
    }

    return result;
}

int VectorGetIndex(const char* string, std::vector<std::string> searchin)
{
	auto iterator = std::find(searchin.begin(), searchin.end(), string);
	int index = std::distance(searchin.begin(), iterator);

	if(iterator == searchin.end())
		return -1;

	return index;
}

std::string weburl = "https://zyre.club/zyre/";
int latestversion = 0;
int ourversion = 0;


struct serverdata
{
	pid_t pid;
	std::vector <std::string> keys;
	std::vector <std::string> values;
};

std::vector<serverdata*> Servers;
std::vector<std::string> Nodes;

struct WebMemoryStruct
{
	char *memory;
	size_t size;
};


WebMemoryStruct webdata = {0};

static size_t WebWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
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

bool FileWrite(const char*filename, char* content, size_t contentlength)
{
	FILE *fhandle = fopen(filename, "wb");
	
	if(!fhandle)
	{
		return false;
	}

	fwrite(content, sizeof(char), contentlength, fhandle);

	fclose(fhandle);

}

char* FileRead(const char*filename, int* filesize)
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

bool NeedsUpdate()
{
	char cversion[8] = {0};
	sprintf(cversion, "%d", ourversion);

	std::string updateurl = weburl + "autoupdate.php?checkver=1&currentversion=" + cversion;

	if(FetchWebData(updateurl.c_str()))
	{
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

bool DownloadChangelog()
{
	char cversion[8] = {0};
	sprintf(cversion, "%d", ourversion);

	std::string updateurl = weburl + "autoupdate.php?changelog=1&currentversion=" + cversion;

	if(FetchWebData(updateurl.c_str()))
	{
		return;
	}

	printf("Downloading new changelog...\n");

	FileWrite("leyzyre_changelog.txt", webdata.memory, webdata.size);

	printf("Saved leyzyre_changelog.txt\n");

}

bool DownloadUpdate()
{
	char cversion[8] = {0};
	sprintf(cversion, "%d", ourversion);

	std::string updateurl = weburl + "autoupdate.php?currentversion=" + cversion;

	printf("Downloading new worker...\n");

	if(FetchWebData(updateurl.c_str()))
	{
		return;
	}

	FileWrite("leyzyreworker", webdata.memory, webdata.size);
	printf("Saved leyzyreworker\n");

	ourversion = latestversion;

	memset(cversion, 0, 8);
	sprintf(cversion, "%d", ourversion);

	FileWrite("leyzyre_version.txt", cversion, strlen(cversion));

	printf("Saved updated leyzyre_version.txt\n");

}


void KillRunningServers()
{
	for(auto it = Servers.begin(); it != Servers.end(); ++it)
	{
		serverdata* Server = *it;

		if(Server->pid==0)
			continue;

		kill(Server->pid, SIGKILL);
		usleep(100*1000);
		Server->pid = 0;
	}
}

void DeleteMemServers()
{
	KillRunningServers();

	while(Servers.size()>0)
	{
		serverdata *curserv = Servers.back();
		delete curserv;
		curserv = 0;

		Servers.pop_back();
	}
}

 std::vector <std::string> settingkeys;
 std::vector <std::string> settingvalues;

void ParseServerSetting(std::string setting)
{
 	auto strings = explode(setting, '\n');


 	while(strings.size()>0)
 	{
 		std::string curline = strings.back();


 		if(1>curline.length())
 		{
 			strings.pop_back();
 			continue;
 		}

 		//printf("curline: %s\n", curline.c_str());

		auto keyval = explode(curline, '=');
 			
 		//printf("KV: %s=%s\n", keyval.front().c_str(), keyval.back().c_str());
 		settingkeys.push_back(keyval.front());
 		settingvalues.push_back(keyval.back());

	 	strings.pop_back();
 	}


  	int index_serverid = VectorGetIndex("serverid", settingkeys);
	if(index_serverid == -1)
	{
		printf("Index for serverid not found!\n");
		return;
	}

 	int index_key = VectorGetIndex("key", settingkeys);
	if(index_key == -1)
	{
		printf("Index for key not found!\n");
		return;
	}

 	int index_value = VectorGetIndex("value", settingkeys);
	if(index_value == -1)
	{
		printf("Index for value not found!\n");
		return;
	}

	std::string cserverid = settingvalues.at(index_serverid);
	std::string key = settingvalues.at(index_key);
	std::string value = settingvalues.at(index_value);

	int serverid = atoi(cserverid.c_str());

	if(serverid > Servers.size() || !Servers.at(serverid))
	{
		while(serverid+1>Servers.size())
		{
			Servers.push_back(0);
		}

		serverdata *newserv = new serverdata;
 		memset(newserv,0, sizeof(serverdata));

		Servers.at(serverid) = newserv;
	}

	serverdata *serv = Servers.at(serverid);

	if(!serv)
	{
		printf("Couldn't add or find server with ID: %d !\n",serverid);
	}

 	if(!strcmp(key.c_str(), "activated"))
 	{
 		auto parts = explode(value.c_str(), ':');

 		if(parts.size() != 3)
 		{
 			printf("expected different activated pkg size!\n");
 			return;
 		}

 		std::string ip = parts[0];
 		std::string port = parts[1];
 		std::string pass = parts[2];

 		//FIX THE IP PARSING PLS

		struct sockaddr_in sa;
		sa.sin_addr.s_addr = __builtin_bswap32(atoi(ip.c_str()));


		char strip[30] = {0};
		inet_ntop(AF_INET, &(sa.sin_addr), strip, INET_ADDRSTRLEN);
 		serv->keys.push_back("ip");
 		serv->values.push_back(strip);

 		serv->keys.push_back("port");
 		serv->values.push_back(port);


 		serv->keys.push_back("password");
 		serv->values.push_back(pass);

 	}


 	serv->keys.push_back(key.c_str());
 	serv->values.push_back(value.c_str());

 	printf("FOR SERVER [%d]: %s = %s\n", serverid, key.c_str(), value.c_str());

}


void ParseOtherNodes(std::string input)
{

	FileWrite("leyzyre_nodes.txt", input.c_str(), input.length());

}

void ParseServerSettings(std::string input)
{

	FileWrite("leyzyre_serversettings.txt", input.c_str(), input.length());
 	auto strings = explode(input, ';');
 	while(strings.size()>0)
 	{
 		std::string curservercfg = strings.back();


 		if(1>curservercfg.length())
 		{
 			strings.pop_back();
 			continue;
 		}

		settingkeys.clear();
		settingvalues.clear();

 		ParseServerSetting(curservercfg);


	 	strings.pop_back();
 	}



}
void UpdateServers()
{
	printf("Clearing old server setting data\n");
	DeleteMemServers();


	std::string serversurl = weburl + "/serverdata.php";

	printf("Downloading servers...\n");

	if(FetchWebData(serversurl.c_str()	))
	{
		return;
	}

	if(webdata.size==0)
	{
		printf("No servers!\n");
		return;
	}


 	std::string s = webdata.memory;

 	size_t find = s.find("<br>");

 	while(find != std::string::npos)
 	{
  		s.replace(find, 4, "\n");
		find = s.find("<br>");
 	}

 	printf("Downloaded servers, parsing!\n");


 	auto strings = explode(s, '?');



 	ParseOtherNodes(strings.front());
 	ParseServerSettings(strings.back());


	//FileWrite("servers.txt",s.c_str(), s.length());
	printf("Saved servers and their settings.\n");


}




void CheckServerOnline()
{
	std::string last_ip = "";
	
	for(auto it = Servers.begin(); it != Servers.end(); ++it)
	{
		serverdata* Server = *it;

		if(!Server)
			continue;

		std::string serverip = "";
		std::string serverport = "";
		std::string a2sport = "";
		std::string proxyport = "";
		std::string password = "";

		std::string proxyip = "";
		std::string offset = "";
		std::string extraplayers = "";
		std::string customname = "";

		int index_ip = VectorGetIndex("ip", Server->keys);
		int index_port = VectorGetIndex("port", Server->keys);
		int index_a2sport = VectorGetIndex("a2sport", Server->keys);
		int index_proxyport = VectorGetIndex("proxyport", Server->keys);
		int index_password = VectorGetIndex("password", Server->keys);

		int index_proxyip = VectorGetIndex("proxyip", Server->keys);
		int index_offset = VectorGetIndex("idoffset", Server->keys);
		int index_extraplayers = VectorGetIndex("extraplayers", Server->keys);
		int index_customname = VectorGetIndex("customname", Server->keys);

		if(index_ip == -1)
		{
			printf("Index for IP not found!\n");
			continue;
		}

		serverip = Server->values.at(index_ip);


		if(index_port == -1)
		{
			printf("sip: %s ; ", serverip.c_str());

			printf("Index for Port not found!\n");
			continue;
		}


		serverport = Server->values.at(index_port);

		if(index_a2sport == -1)
		{
			printf("sip: %s:%s; ", serverip.c_str(), serverport.c_str());

			printf("Index for A2SPort not found!\n");
			continue;
		}

		if(index_proxyport == -1)
		{
			//printf("Index for Proxyport not found!\n");
			index_proxyport = index_a2sport;
		}


		if(index_password == -1)
		{
			printf("sip: %s:%s; ", serverip.c_str(), serverport.c_str());

			printf("Index for Password not found!\n");
			continue;
		}

		if(index_proxyip != -1)
		{
			proxyip =  Server->values.at(index_proxyip);
		}
		
		if(index_offset != -1)
		{
			offset = Server->values.at(index_offset);
		}
		
		if(index_extraplayers != -1)
		{
			extraplayers = Server->values.at(index_extraplayers);
		}

		if(index_customname != -1)
		{
			customname = Server->values.at(index_customname);
		}


		a2sport = Server->values.at(index_a2sport);
		proxyport = Server->values.at(index_proxyport);
		password = Server->values.at(index_password);

		//printf("serverip: %s\n", serverip.c_str());

		if(Server->pid != 0)
		{

			int status = waitpid(Server->pid, -1, WNOHANG);

			if (status != 0)
			{
				printf("child is rip\n");
				kill(Server->pid, SIGKILL);
				Server->pid = 0;
				usleep(100*1000);
				continue;
			}


			//printf("its alive\n");
			continue;
		}

		const char*filename = "leyzyreworker";

		long int t = (long int)time(NULL);

		int penis = rand() % 4;

		int nicenum = 0xDAD00000;

		if(penis == 1)
			nicenum = 0xFED00000;

		if(penis == 2)
			nicenum = 0xFAF00000;

		if(penis == 3)
			nicenum = 0xCBF00000;

		nicenum = nicenum | (rand() % 0xCCCCC);

		char arg0[0xFF] = {0};
		sprintf(arg0, "%d\"%d\"\n", t, nicenum);


		char **argv = new char*[20];

		std::string arg1 = serverip + ":" + serverport;

		argv[0] = arg0;
		argv[1] = arg1.c_str();
		argv[2] = a2sport.c_str();
		argv[3] = proxyport.c_str();
		argv[4] = password.c_str();
		argv[5] = offset.c_str();
		argv[6] = extraplayers.c_str();
		argv[7] = customname.c_str();
		argv[8] = proxyip.c_str();
		argv[9] = 0;

		pid_t i = fork();

	    if (0>i)
	    {
	    	printf("Creating child process failed: %d !\n", errno);
	    	continue;
	    }

	    if(i==0)
	    {
	    	printf("Inserting workercode into child..\n");
	        execve(filename, argv, NULL);
	        _exit(1);
	        return;
	    }

		Server->pid = i;
	    printf("Created child process: %d\n", (int)i);

		if(serverip.compare(last_ip) == 0)
		{
			//usleep(700*1000);
		}
		
		usleep(1200*1000);

		last_ip = serverip;
	}


}




int main(int argc, char **argv)
{
	
	usleep((rand()%5 * 100)*1000);

	curl_global_init(CURL_GLOBAL_ALL);

	char*readourversion = FileRead("leyzyre_version.txt", 0);

	if(readourversion)
	{
		ourversion = atoi(readourversion);

		printf("Zyre | Revision: %d\n", ourversion);
	}else{
		printf("Zyre | No version saved.\n");
	}


	printf("=== Checking for updates...\n\n");


	delete[] readourversion;
	readourversion = 0;

	bool justloaded = true;

	static int checkwebcounter = 0;
	bool shouldcheckweb = true;

	while(1)
	{
		checkwebcounter = checkwebcounter - 1;

		if(1>checkwebcounter)
		{
			checkwebcounter = WEBCHECKFREQUENCY;
			shouldcheckweb = true;
		}

		if(shouldcheckweb)
		{
			shouldcheckweb = false;
			bool needsupdate = NeedsUpdate();

			if(needsupdate)
			{
				printf("=== Updating to revision %d ...\n\n", latestversion);
				KillRunningServers();
				DownloadChangelog();
				DownloadUpdate();
				UpdateServers();

				usleep(1000*1000);

				printf("=== Updater done\n\n");

			}else{

				if(justloaded)
				{
					KillRunningServers();
					UpdateServers();

					usleep(1000*1000);

					printf("=== Updater done\n\n");
				}
			}
		}	

		CheckServerOnline();

		justloaded = false;
		usleep(1*1000*1000);
	}

	curl_global_cleanup();

	return 0;
}