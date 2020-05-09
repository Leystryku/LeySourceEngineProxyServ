
#include <stdio.h>
#include <string.h>
#include <chrono>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <cmath>

#include <maxminddb.h>

#include "a2s.h"
#include "workerdata.h"
#include "leybuf.h"

MMDB_s mmdb;

using std::chrono::steady_clock;

A2SServer::~A2SServer()
{
	
	for(uint32_t i=0;i<A2S_MASTERSERVERCOUNT;i++)
	{
		udp->SendTo(imasterservs[i], 27011, "\x71", 1);
		printf("Deleting a2sserv for: %i\n", (int)i);
	}

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

	if(sameportproxy)
	{
		delete sameportproxy;
		sameportproxy = 0;
	}

}

MMDB_lookup_result_s GEO_GetResult(uint32_t ip)
{
	static int reloadcount = 0;
	reloadcount = reloadcount + 1;

	if(reloadcount == 1000)
	{
		printf("reload\n");

		MMDB_close(&mmdb);

		int status = MMDB_open("ipdb.mmdb", MMDB_MODE_MMAP, &mmdb);

	    if (MMDB_SUCCESS != status)
		{
			for(int i=0;i<10;i++)
			{
				status = MMDB_open("ipdb.mmdb", MMDB_MODE_MMAP, &mmdb);

				if(MMDB_SUCCESS == status)
					break;

				fprintf(stderr, "\n  Can't open ipdb.mmdb\n", MMDB_strerror(status));

		        if (MMDB_IO_ERROR == status)
		        {
		            fprintf(stderr, "    IO error: %s\n", strerror(errno));
		        }

		        usleep(3000 * 1000 );
			}
	    }
	    reloadcount = 0;
	}
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip;
	addr.sin_port = 0;
	
	int mmdb_error = 0;
	MMDB_lookup_result_s result = MMDB_lookup_sockaddr(&mmdb, (sockaddr *)&addr, &mmdb_error);
	
	if (MMDB_SUCCESS  != mmdb_error)
	{
		printf("!GeoERR! Lookup in DB failed %x.\n", ip);
	}
	
	return result;
}

int get_long_and_lat(uint32_t lookip, double *longitude, double *latitude)
{
	
	if(!latitude || !longitude)
	{
		return 1;
	}
	*latitude = 0;
	*longitude = 0;
	
	MMDB_lookup_result_s  result = GEO_GetResult(lookip);
	
	if(!result.found_entry)
	{
		//printf("!GeoERR! Could not find entry.\n");
		return 1;
	}
				
	MMDB_entry_data_s entry_data;
	int res = MMDB_get_value(&result.entry, &entry_data, "location", "longitude", NULL);
	
	if (MMDB_SUCCESS != res)
	{
		//printf("!GeoERR! Bad IP!\n");
		return 1;
	}
	if (!entry_data.has_data)
	{	
		//printf("!GeoERR! No Data found!\n");
		return 1;
	}

	double rlongitude = entry_data.double_value;
	MMDB_get_value(&result.entry, &entry_data, "location", "latitude", NULL);

	double rlatitude = entry_data.double_value;

	//printf("GEOPOS: (%f:%f)\n", (float)rlatitude, (float)rlongitude);

	*longitude = rlongitude;
	*latitude = rlatitude;

	return 0;
}


std::vector<std::string> split(const std::string& s, char delimiter)
{
   std::vector<std::string> tokens;
   std::string token;
   std::istringstream tokenStream(s);
   while (std::getline(tokenStream, token, delimiter))
   {
      tokens.push_back(token);
   }
   return tokens;
}

A2SServer::A2SServer(WorkerData *setworker)
{
    int status = MMDB_open("ipdb.mmdb", MMDB_MODE_MMAP, &mmdb);

    if (MMDB_SUCCESS != status)
	{
        fprintf(stderr, "\n  Can't open ipdb.mmdb\n", MMDB_strerror(status));

        if (MMDB_IO_ERROR == status) {
            fprintf(stderr, "    IO error: %s\n", strerror(errno));
        }
        exit(1);
    }

	srand((int32_t)clock() + rand()%100 + (int32_t)time(0));
	memset(a2srecvdata, 0, A2S_MAXDATASIZE);
	memset(a2ssenddata, 0, A2S_MAXDATASIZE);
	wearemaster = false;
	
	recvsize = 0;

	worker = new WorkerData;
	memcpy(worker, setworker, sizeof(WorkerData));
	memset(&splitpackets, 0, sizeof(splitpackets));

	memset(recvip, 0, 30);
	
	justinited = 1;
	sameportproxy = 0;
	curmasterserv = rand() % 4;


	FILE *fnodestorage = fopen("leyzyre_nodes.txt", "rb");
	fseek(fnodestorage, 0, SEEK_END);
	size_t len = ftell(fnodestorage);
	
	
	rewind(fnodestorage);
	
	char *readbuf = new char[len+1];
	
	memset(readbuf, 0, len+1);
	fread(readbuf, sizeof(char), len, fnodestorage);
	fclose(fnodestorage);
	
	
	std::string sread = "";
	sread.append(readbuf);
	
	
	delete[] readbuf;
	
	std::vector<std::string> tokens = split(sread, ';');
	
	
	for (std::vector<std::string>::iterator jt = tokens.begin() ; jt != tokens.end(); ++jt)
	{
		std::string curipblock = *jt;
		if(!strncmp(curipblock.c_str(), "ourip", 5))
		{
			curipblock.erase(0, 6);
			curipblock.erase(curipblock.length()-1, 1);
			
			ournodeip = strtoul(curipblock.c_str(), NULL, 0);
			continue;
		}
		
		node newnode;
		newnode.id = 0;
		
		std::vector<std::string> curlines = split(curipblock, '\n');
	
		for (std::vector<std::string>::iterator it = curlines.begin() ; it != curlines.end(); ++it)
		{
			std::string curline  = *it;
			std::vector<std::string> parseln = split(curline, '=');
			
			if(2>parseln.size())
			{
				continue;
			}
			std::string key = parseln[0];
			std::string value = parseln[1];
			
			if(!key.compare("ismaster"))
			{
				if(value.c_str()[0] == '1')
				{
					newnode.ismaster = true;
				}else{
					newnode.ismaster = false;
				}
				
				continue;
			}
			
			if(!key.compare("nodeid"))
			{
				newnode.id = strtoul(value.c_str(), NULL, 0);
				continue;
			}
			
			if(!key.compare("ip"))
			{
				newnode.ip = strtoul(value.c_str(), NULL, 0);
				

				continue;
			}
			
			if(!key.compare("lng"))
			{
				newnode.longitude = atof(value.c_str());
				continue;
			}

			if(!key.compare("lat"))
			{
				newnode.latitude = atof(value.c_str());
				continue;
			}

			if(!key.compare("name"))
			{
				strcpy(newnode.name, value.c_str());
				continue;
			}
			
		}
		
		if(newnode.id != 0)
		{
			nodes.push_back(newnode);
		}
		
	}
	
	for(int i=0;i<nodes.size();i++)
	{
		if(nodes[i].ip == ournodeip)
		{
			nodes[i].isournode = true;
			if (nodes[i].ismaster)
			{
				wearemaster = true;
			}
			
		}else{
			nodes[i].isournode = false;
		}					

	}
	
	printf("Our IP: %lu\n", ournodeip);
	
	
}

void A2SServer::WriteProxyHeader(leybuf *towrite, uint32_t numip, uint16_t port, uint8_t flags)
{

	towrite->WriteInt64(worker->ipassword);
	towrite->WriteInt32(numip);
	towrite->WriteInt16(port);
	towrite->WriteChar(flags);
}

void A2SServer::RequestSourceServerInfo()
{
	leybuf writedata(a2ssenddata, A2S_MAXDATASIZE);
	this->WriteProxyHeader(&writedata, -1, -1, 0);

	writedata.WriteInt32(-1);
	writedata.WriteChar('T');
	writedata.WriteString("Source Engine Query");

	udp->SendTo(this->worker->str_srcip.c_str(), this->worker->isrcport, writedata.GetData(), writedata.GetPos());
}

void A2SServer::RequestSourceServerA2SPlayers()
{
	leybuf writedata(a2ssenddata, A2S_MAXDATASIZE);
	this->WriteProxyHeader(&writedata, -1, -1, 0);

	writedata.WriteInt32(-1);
	writedata.WriteChar('U');
	writedata.WriteInt32(-1);

	udp->SendTo(this->worker->str_srcip.c_str(), this->worker->isrcport, writedata.GetData(), writedata.GetPos());
}

void A2SServer::RequestSourceServerA2SPlayersWChan()
{
	leybuf writedata(a2ssenddata, A2S_MAXDATASIZE);
	this->WriteProxyHeader(&writedata, -1, -1, 0);

	writedata.WriteInt32(-1);
	writedata.WriteChar('U');
	writedata.WriteInt32(this->plychallenge);

	udp->SendTo(this->worker->str_srcip.c_str(), this->worker->isrcport, writedata.GetData(), writedata.GetPos());
}

void A2SServer::RequestMasterServerChallenge()
{
	if(curmasterserv>A2S_MASTERSERVERCOUNT)
	{
		curmasterserv = 0;
	}
		
	uint32_t i = (uint32_t)curmasterserv;

	udp->SendTo(imasterservs[i], 27011, "\x71", 1);
	printf("A challenge is being requested from: [%i] - %s\n", i+1, masterservs[i]);

	curmasterserv += 1;
}

void A2SServer::RequestMasterDataIfNeeded()
{
	steady_clock::time_point curtime = steady_clock::now();

  	auto chrono_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastrefreshtimemaster);
	auto mseconds = chrono_milliseconds.count();

	if(A2S_REFRESHTIMEMASTER>mseconds || justinited)
	{
		return;
	}
	
	lastrefreshtimemaster = steady_clock::now();

	
	if(worker->nomaster||justinited)
		return;
	

	printf("Master Refreshes are being requested!\n");

	this->RequestMasterServerChallenge();

}

void A2SServer::RequestInfoDataIfNeeded()
{
	steady_clock::time_point curtime = steady_clock::now();

  	auto chrono_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastrefreshtime);
	auto mseconds = chrono_milliseconds.count();

	if(A2S_REFRESHTIME>mseconds && justinited != 2)
	{
		return;
	}
	
	lastrefreshtime = steady_clock::now();

	if(justinited)
	{
		justinited = 3;
	}

	
	printf("Info Refreshes are being requested!\n");


	this->RequestSourceServerInfo();


}

void A2SServer::RequestPlayerDataIfNeeded()
{
	steady_clock::time_point curtime = steady_clock::now();

  	auto chrono_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastrefreshtime_a2sply);
	auto mseconds = chrono_milliseconds.count();

	if(A2S_REFRESHTIMEPLY>mseconds || justinited)
	{
		return;
	}
	
	lastrefreshtime = steady_clock::now();
	
	auto chrono_milliseconds_a2sply = std::chrono::duration_cast<std::chrono::milliseconds>(curtime - lastrefreshtime_a2sply);
	auto mseconds_a2sply = chrono_milliseconds_a2sply.count();
	
	if(justinited)
	{
		justinited = 3;
	}

	
	printf("Refreshes are being requested!\n");


	this->RequestSourceServerInfo();
	
	if(A2S_REFRESHTIMEPLY<mseconds_a2sply || justinited)
	{
		lastrefreshtime_a2sply = steady_clock::now();
		
		printf("Requesting player list\n");
		this->RequestSourceServerA2SPlayers();
		
	}
	
	if(worker->nomaster||justinited)
		return;
	

	this->RequestMasterServerChallenge();

}

void A2SServer::OnReceiveNoData() // if we're blocking this'll be called way less
{

	if(sameportproxy)
	{
		sameportproxy->OnReceiveNoData();
	}

}

void A2SServer::OnReceiveMasterServerData()
{

	leybuf readmasterdata(this->a2srecvdata, this->recvsize);
	uint32_t oob = readmasterdata.ReadInt32();



	if(oob != -1)
	{
		printf("Master server packet has invalid oob header: %d\n", oob);
		return;
	}

	uint8_t type = readmasterdata.ReadChar();

	if(type != 's')
	{
		printf("Unknown message received from master server: %s, header: %i ( %c )\n msg: %s\n", this->recvip, oob, type, this->a2srecvdata);
		return;
	}

	if(readmasterdata.IsOverflowed())
	{
		return;
	}

	char whatever = (char)readmasterdata.ReadChar();

	if(readmasterdata.IsOverflowed())
	{
		return;
	}

	int32_t masterchallenge = (int32_t)readmasterdata.ReadInt32();
					
	char data[800] = {0};
	
	char safeservername[0x1FF] = {0};
	char safekeywords[0x1FF] = {0};
	
	for(int i=0;i<this->servera2s.name.length();i++)
	{
		if(i>0x1FA)
		{
			break;
		}
		safeservername[i] = this->servera2s.name.c_str()[i];
		
		if(!safeservername[i])
		{
			safeservername[i] = ' ';
		}
	}

	
	for(int i=0;i<this->servera2s.edf_keywords.length();i++)
	{
		if(i>0x1FA)
		{
			break;
		}
		
		safekeywords[i] = this->servera2s.edf_keywords.c_str()[i];
		
		if(!safekeywords[i])
		{
			safekeywords[i] = ' ';
		}
	}
	
	static int32_t steamidpart = 10000 + worker->proxyport;
	
	static int32_t masterply = 29;
	masterply = masterply + 1;
	if(masterply > 110)
	{
		masterply = 29;
	}

	sprintf(data, "%c%c\\protocol\\7\\challenge\\%i\\gameport\\%d\\steamid\\901111000001%d\\players\\%i\\max\\%i\\bots\\%i\\gamedir\\%s\\map\\%s\\password\\0\\os\\w\\lan\\0\\region\\0\\type\\d\\secure\\1\\version\\%s\\product\\garrysmod\\dedicated\\true\\gametype\\%s\\name\\%c%s%c", 0x30, 0x0A, masterchallenge, (int)worker->gameport, steamidpart, masterply, 120, 0, this->servera2s.folder.c_str(), this->servera2s.map.c_str(), this->servera2s.version.c_str(), safekeywords, 'A' + rand() % 20, safeservername, 0x0A);


	uint32_t len = 100;

	while (data[len] != '\x0A')
		len++;

					
	len++;


	if(worker->nomaster)
	{
		return;	
	}

	udp->SendTo(udp->lastiip, this->recvport, data, len);
	printf("master challenge received:  %i \n", masterchallenge);
	//printf(" | responding (resp len: %i) :: %s\n", len , data);

}

int32_t A2SServer::OnReceiveSourceServerA2SData()
{
	if(justinited)
	{
		justinited = 0;
		this->RequestMasterServerChallenge();
	}

	if(8+4+2+1 > this->recvsize)
	{
		//printf("data too small\n");

		return 1;
	}

	leybuf readsourceserverdata(this->a2srecvdata, this->recvsize);


	int64_t password = readsourceserverdata.ReadInt64();

	if(password != worker->ipassword)
	{
		#ifdef DEBUG
		printf("Wrong password from: %s", this->recvip);
		#endif
		return 1;
	}

	int32_t toip = readsourceserverdata.ReadInt32();
	int16_t toport = readsourceserverdata.ReadInt16();


	if(readsourceserverdata.IsOverflowed() || toip != (int32_t)-1 || toport != (int16_t)-1)
	{
		return 1;
	}

	int8_t flags = readsourceserverdata.ReadChar();



	int32_t oob = readsourceserverdata.ReadInt32();
	
	if(oob == (int32_t)-3)
	{
		return 1;
	}
	
	if( oob == (int32_t)-2)
	{
		
		if(splitpackets_i >= sizeof(splitpackets) / sizeof(SourceSplitPacket))
		{
			splitpackets_i = 0;
		}
		
		int32_t id = readsourceserverdata.ReadInt32();
		uint8_t total = readsourceserverdata.ReadChar();
		uint8_t number = readsourceserverdata.ReadChar();
		int16_t packetsize = readsourceserverdata.ReadInt16();

		if(splitpackets[0].packetsize > 0 && splitpackets[0].id != id)
		{
			splitpackets_i = 0;
		}
		
		if(packetsize > 2048 || number > sizeof(splitpackets) / sizeof(SourceSplitPacket))
		{
			return 0;
		}
		
		splitpackets[number].id = id;
		splitpackets[number].total = total;
		splitpackets[number].number = number;
		splitpackets[number].packetsize = packetsize;
		
		memset(splitpackets[number].msg, 0, sizeof(splitpackets[number].msg));
		
		readsourceserverdata.ReadBytes(splitpackets[number].msg, splitpackets[number].packetsize);
		
		
		
		if(splitpackets_i+1 == total)
		{
			char *fullpacket = new char[0xFFFFF];
			
			int32_t fsize = 0;
			
			for(int i=0;i<splitpackets_i;i++)
			{
				if(fsize + splitpackets[i].packetsize + 8 + 4 + 2 + 1> 0xFFFFF)
				{
					return 0;
				}
				
				memcpy((char*)fullpacket + 8 + 4 + 2 + 1 + fsize, splitpackets[i].msg, splitpackets[i].packetsize);
				fsize = fsize + splitpackets[i].packetsize;		
			}
			
			this->recvsize = fsize;
			
			memcpy((char*)fullpacket, this->a2srecvdata, 8+4+2+1);
			memcpy(this->a2srecvdata, fullpacket, fsize);
			
			
			splitpackets_i  = 0;
			delete[] fullpacket;
			
			int32_t ret = this->OnReceiveSourceServerA2SData();
			
			
			return ret;
		}else{
			//printf("Split packet: %d __ %d __ %d __ %d\n", (int32_t)splitpackets[splitpackets_i].id, (int32_t)splitpackets[splitpackets_i].total, (int32_t)splitpackets[splitpackets_i].number, (int32_t)splitpackets[splitpackets_i].packetsize);
			
			splitpackets_i += 1;
		}
		
		return 0;
	}
	
	if(oob == (int32_t)-1)
	{
		uint8_t type = readsourceserverdata.ReadChar();

		if(type != (uint8_t)'I' && type != (uint8_t)'A' && type != (uint8_t)'D')
		{
			printf("Unknown message received from source server: %s, header: %i ( %x )\n msg: %s\n", this->recvip, oob, type, this->a2srecvdata);
			return 1;
		}

		if(type == (uint8_t)'I')
		{
			this->OnReceiveSourceServerA2SInfo();
			return 0;
		}
		
		if(type == (uint8_t)'A')
		{
			this->OnReceiveSourceServerA2SPlyChallenge();
			return 0;
		}
		
		if(type == (uint8_t)'D')
		{
			this->OnReceiveSourceServerA2SPlayers();
			return 0;
		}
	
	}
	
	return 1;

}

void A2SServer::OnReceiveSourceServerA2SInfo()
{
	memcpy(this->srca2sinfo, this->a2srecvdata, this->recvsize);

	leybuf reada2sinfo(this->a2srecvdata, this->recvsize);
	
	reada2sinfo.ReadInt64();
	reada2sinfo.ReadInt32();
	reada2sinfo.ReadInt16();
	reada2sinfo.ReadChar();

	reada2sinfo.ReadInt32();

	if(reada2sinfo.IsOverflowed())
		return;
	

	servera2s.header = reada2sinfo.ReadChar();
	servera2s.protocol = reada2sinfo.ReadChar();


	reada2sinfo.ReadString(servera2s.name);
	reada2sinfo.ReadString(servera2s.map);
	reada2sinfo.ReadString(servera2s.folder);
	reada2sinfo.ReadString(servera2s.game);
	servera2s.appid = reada2sinfo.ReadInt16();

	char readplayers = reada2sinfo.ReadChar();

	if(readplayers >= servera2s.players)
	{
		servera2s.players = readplayers;
	}else{
		if(plyreducecnt>0)
		{
			plyreducecnt--;
		}else{
			plyreducecnt = 3;
			servera2s.players -= 1;
			if(0>servera2s.players)
			{
				servera2s.players = 0;
			}
		}
	}

	servera2s.maxplayers = reada2sinfo.ReadChar();
	servera2s.bots = reada2sinfo.ReadChar();
	
	servera2s.servertype = reada2sinfo.ReadChar();
	servera2s.enviroment = reada2sinfo.ReadChar();
	servera2s.visibility = reada2sinfo.ReadChar();

	if(reada2sinfo.IsOverflowed())
		return;

	servera2s.vac = reada2sinfo.ReadChar();
	reada2sinfo.ReadString(servera2s.version);
	servera2s.edf = reada2sinfo.ReadChar();

	if(!(servera2s.edf & 0x80))
	{
		servera2s.edf = servera2s.edf | 0x80;
	}

	if(servera2s.edf & 0x80) // game port
	{
		//servera2s.edf_gameport = reada2sinfo.ReadInt16();
		reada2sinfo.ReadInt16();
		servera2s.edf_gameport = (short)worker->gameport;

	}
	if(servera2s.edf & 0x10) // server sid
	{
		servera2s.edf_serversid = reada2sinfo.ReadInt64() ;
		//90129626372196352llu
		if(true || worker->sidoffset == 0)
		{
			servera2s.edf_serversid += (short)worker->proxyport;//+= rand() % 0xFFFF;
		}else{
			servera2s.edf_serversid += worker->sidoffset;
		}
		
	}

	if(servera2s.edf & 0x40) // source TV
	{
		servera2s.edf_sourcetv_port = reada2sinfo.ReadInt16();
		reada2sinfo.ReadString(servera2s.edf_sourcetv_name);
	}

	if(servera2s.edf & 0x20) // keywords
	{
		reada2sinfo.ReadString(servera2s.edf_keywords);
	}

	if(servera2s.edf & 0x01) // gameid
	{
		servera2s.edf_servergameid = reada2sinfo.ReadInt64();
	}

	printf("numclients:%d maxclients:%d bots: %d\n", (int32_t)servera2s.players, (int32_t)servera2s.maxplayers, (int32_t)servera2s.bots);
	printf("map:%s gamefolder: %s\n", servera2s.map.c_str(), servera2s.folder.c_str());

	leybuf writea2sinforeply(this->srca2sinfo, A2S_MAXDATASIZE);

	writea2sinforeply.WriteInt32(-1);

	writea2sinforeply.WriteChar(servera2s.header);
	writea2sinforeply.WriteChar(servera2s.protocol);

	
	if(worker->customname.length()>0)
	{
		servera2s.name = "";
		servera2s.name = worker->customname.c_str();
	}
	
	writea2sinforeply.WriteString( servera2s.name);
	

	writea2sinforeply.WriteString(servera2s.map);
	writea2sinforeply.WriteString(servera2s.folder);
	writea2sinforeply.WriteString(servera2s.game);

	writea2sinforeply.WriteInt16(servera2s.appid);
	
	if(worker->extraplayers)
	{
		uint8_t fakedplayers = servera2s.players + worker->extraplayers;

		if(fakedplayers > servera2s.maxplayers)
		{
			fakedplayers = servera2s.maxplayers - 1;
		}

		writea2sinforeply.WriteChar(fakedplayers);
	}else{
		writea2sinforeply.WriteChar(servera2s.players);
	}

	writea2sinforeply.WriteChar(servera2s.maxplayers);
	writea2sinforeply.WriteChar(servera2s.bots);

	writea2sinforeply.WriteChar(servera2s.servertype);
	writea2sinforeply.WriteChar(servera2s.enviroment);
	writea2sinforeply.WriteChar(servera2s.visibility);
	writea2sinforeply.WriteChar(servera2s.vac);


	writea2sinforeply.WriteString(servera2s.version);
	writea2sinforeply.WriteChar(servera2s.edf);

	if(servera2s.edf & 0x80) // game port
	{
		writea2sinforeply.WriteInt16(servera2s.edf_gameport);
	}

	if(servera2s.edf & 0x10) // server sid
	{
		writea2sinforeply.WriteInt64(servera2s.edf_serversid);

	}

	if(servera2s.edf & 0x40) // source TV
	{
		writea2sinforeply.WriteInt16(servera2s.edf_sourcetv_port);
		writea2sinforeply.WriteString(servera2s.edf_sourcetv_name);
	}

	if(servera2s.edf & 0x20) // keywords
	{
		writea2sinforeply.WriteString(servera2s.edf_keywords);
	}

	if(servera2s.edf & 0x01) // gameid
	{
		writea2sinforeply.WriteInt64(servera2s.edf_servergameid);
	}


	srca2sinfo_len = writea2sinforeply.GetPos();


}

void A2SServer::OnReceiveSourceServerA2SPlyChallenge()
{
	leybuf reada2sply(this->a2srecvdata, this->recvsize);
	
	reada2sply.ReadInt64();
	reada2sply.ReadInt32();
	reada2sply.ReadInt16();
	reada2sply.ReadChar();

	reada2sply.ReadInt32();

	reada2sply.ReadChar();
	
	if(reada2sply.IsOverflowed())
		return;

	this->plychallenge = reada2sply.ReadInt32();
	
	
	this->RequestSourceServerA2SPlayersWChan();
}

void A2SServer::OnReceiveSourceServerA2SPlayers()
{
	leybuf reada2sply(this->a2srecvdata, this->recvsize);
	
	reada2sply.ReadInt64();
	reada2sply.ReadInt32();
	reada2sply.ReadInt16();
	reada2sply.ReadChar();

	reada2sply.ReadInt32();

	if(reada2sply.IsOverflowed())
		return;

	uint8_t header = reada2sply.ReadChar();
	
	uint8_t players = (uint8_t)reada2sply.ReadChar();
	

	memset(&servera2sply, 0, sizeof(servera2sply));
	
	servera2sply.players = players;
	
	for(uint8_t i=0;i<players;i++)
	{
		char readnick[0xFF] = {0};
		
		reada2sply.ReadChar();
		reada2sply.ReadString(readnick, 0xFF);
		
		strcpy(servera2sply.names[i], readnick);
		servera2sply.frags[i] = (long)reada2sply.ReadInt32();
		servera2sply.connectiontimes[i] = reada2sply.ReadFloat();
	}
	
	
}

int32_t A2SServer::OnReceiveClientData()
{
	leybuf readsourceserverdata(this->a2srecvdata, this->recvsize);
	int32_t oob = readsourceserverdata.ReadInt32();
	uint8_t type = readsourceserverdata.ReadChar();

	if(readsourceserverdata.IsOverflowed())
		return 1;

	if(oob != -1)
	{
		#ifdef DEBUG
		printf("Unknown message received from client: %s, header: %i ( %c )\n msg: %s\n", this->recvip, oob, type, this->a2srecvdata);
		#endif
		return 1;
	}

	if(type == 'T' && !worker->a2signore)
	{

		double reqlat = 0;
		double reqlong = 0;

		int failed = get_long_and_lat(udp->lastiip, &reqlong, &reqlat);
		
		if(failed)
		{
			if(this->wearemaster)
			{
				this->OnReceiveClientReqA2SInfo();
			}
			
		}else{
			int32_t winnernode = 0;
			double winnernodedist = 10000;
			
			for(int32_t i=0;i<nodes.size();i++)
			{
				node curnode = nodes[i];
				
				double distx = (curnode.latitude - reqlat);
				double disty = (curnode.longitude - reqlong);
				distx = distx * distx;
				disty = disty * disty;
				
				double distp = sqrt(distx + disty);
				
				if(winnernodedist > distp)
				{
					winnernodedist = distp;
					winnernode = i;
				}
			}
			
			node winner = nodes[winnernode];
			
			if(winner.isournode)
			{
				this->OnReceiveClientReqA2SInfo();
			}
			
			
			return 0;
		}

		return 0;
	}

	if(type == 'U')
	{
		this->OnReceiveClientReqA2SPlayers();
		return 0;
	}
	
	if(type == 'q' || type == 'k')
	{
		return 1;
	}
	
	//|| type != 'T' && type != 'U' || type != 'V' || type == 'W' || type != 'i'
	

	
	//V is A2S_RULES
	
	//W is A2S_SERVERQUERY_GETCHALLENGE
	
	//'i' is A2A_PING
	

	return 0;

}

void A2SServer::OnReceiveClientReqA2SInfo()
{
	#ifdef DEBUG
	printf("Replied with a2s_info to: %s\n", this->recvip);
	#endif
	udp->SendTo(udp->lastiip, this->recvport, srca2sinfo, srca2sinfo_len);
}

void A2SServer::OnReceiveClientReqA2SPlayers()
{


	leybuf readsourceserverdata(this->a2srecvdata, this->recvsize);
	readsourceserverdata.ReadInt32();
	readsourceserverdata.ReadChar();
	
	if(readsourceserverdata.IsOverflowed())
		return 1;

	long clientchallenge = (long)readsourceserverdata.ReadInt32();
	
	if(clientchallenge == -1)
	{
		#ifdef DEBUG
		printf("Replied with a2s_players_clientchallenge to: %s\n", this->recvip);
		#endif
		leybuf writedata(a2ssenddata, A2S_MAXDATASIZE);

		writedata.WriteInt32(-1);
		writedata.WriteChar('A');
		writedata.WriteInt32(rand() % 0x7FFFFFF);
	
		udp->SendTo(udp->lastiip, this->recvport, writedata.GetData(), writedata.GetPos());

		this->OnReceiveClientReqA2SInfo();
		return;
	}
	#ifdef DEBUG
	printf("Replied with a2s_players to: %s\n", this->recvip);
	#endif
	if(lastsendlistupdate==-1)
	{
		a2sply_firstbuild  = steady_clock::now();
		lastsendlistupdate = 0;
	}
	
   	auto current_time = steady_clock::now();
   	auto time_sincestart = std::chrono::duration_cast<std::chrono::seconds>(current_time - a2sply_firstbuild).count();

	

   	if(lastsendlistupdate != time_sincestart)
   	{
		int diff = time_sincestart - lastsendlistupdate;
		lastsendlistupdate = time_sincestart;
		
   		leybuf writea2splayersreply(this->srca2splayer, A2S_MAXDATASIZE);
		writea2splayersreply.WriteInt32(-1);
		writea2splayersreply.WriteChar('D');
		
		uint8_t useplayers = servera2sply.players;
		
		if(servera2sply.players > servera2s.players + servera2s.bots)
		{
			useplayers = servera2s.players + servera2s.bots;
		}
		
		writea2splayersreply.WriteChar((char)((int)useplayers));
		
		
			
		for(uint8_t i=0;i<useplayers;i++)
		{
			if(diff > 0)
			{
				servera2sply.connectiontimes[i] += (float)diff;
			}
			writea2splayersreply.WriteChar((char)((uint8_t)i));
			writea2splayersreply.WriteString(servera2sply.names[i]);
			writea2splayersreply.WriteInt32((int)servera2sply.frags[i]);
			writea2splayersreply.WriteFloat(servera2sply.connectiontimes[i]);
		}
		
		srca2splayers_len = writea2splayersreply.GetPos();
   	}

	#define A2S_PLAYER_SPLITEVERY_BYTES 500
	
	if(false && srca2splayers_len > A2S_PLAYER_SPLITEVERY_BYTES)
	{
		uint8_t splitamount = (srca2splayers_len / A2S_PLAYER_SPLITEVERY_BYTES) + 1;
		
		char *sendbuf = new char[2000];
		static int16_t niceid = 0;
		niceid = niceid + 1;
		
		if(0>niceid)
		{
			niceid = 1;
		}
		
		for(uint8_t i=0;splitamount>i;i++)
		{
			/*
			int32_t id = readsourceserverdata.ReadInt32();
			uint8_t total = readsourceserverdata.ReadChar();
			uint8_t number = readsourceserverdata.ReadChar();
			int16_t packetsize = readsourceserverdata.ReadInt16();
			*/
		
			int16_t packetsize = A2S_PLAYER_SPLITEVERY_BYTES;
			int32_t dataoffset = i * packetsize;
			
			if(dataoffset + packetsize > srca2splayers_len)
			{
				packetsize = srca2splayers_len % A2S_PLAYER_SPLITEVERY_BYTES;
			}
			
			
			leybuf wsendbuf(sendbuf, 2000);
			wsendbuf.WriteInt32(-2);
			wsendbuf.WriteInt32(niceid);
			wsendbuf.WriteChar(splitamount+1);
			wsendbuf.WriteChar(i);
			wsendbuf.WriteInt16(packetsize);
			wsendbuf.WriteBytes((char*)this->srca2splayer + dataoffset, packetsize);
			
			udp->SendTo(udp->lastiip, this->recvport, wsendbuf.GetData(), wsendbuf.GetPos());
		}
		//udp->SendTo(udp->lastiip, this->recvport, srca2splayer, srca2splayers_len);
		delete[] sendbuf;
	}else{
		udp->SendTo(udp->lastiip, this->recvport, srca2splayer, srca2splayers_len);

	}
	
}

void A2SServer::OnReceiveData()
{
	bool is_masterreply = false;
	
	for (int i = 0; i < A2S_MASTERSERVERCOUNT; i++)
	{
		if (imasterservs[i] == udp->lastiip)
		{
			this->OnReceiveMasterServerData();
			return;
		}
		
	}

	if (worker->iip == udp->lastiip)
	{
		int32_t worked = this->OnReceiveSourceServerA2SData();

		if(sameportproxy&&worked != 0)
		{
			this->SamePortPatchup();
			sameportproxy->OnReceiveSourceServerProxyData();
		}

		return;
	}


	if(this->justinited)
	{
		#ifdef DEBUG
		printf("Not communicating with clients, didn't init yet!\n");
		#endif
		return;
	}

	int32_t worked = this->OnReceiveClientData();


	if(sameportproxy&&worked != 0)
	{
		this->SamePortPatchup();
		sameportproxy->OnReceiveClientData();
	}

}

int32_t A2SServer::SamePortPatchup()
{
	if(!sameportproxy)
	{
		return 1;
	}

	sameportproxy->recvport = this->recvport;
	sameportproxy->recvsize = this->recvsize;
	strcpy(sameportproxy->recvip, this->recvip);
	memcpy(sameportproxy->proxyrecvdata, this->a2srecvdata, this->recvsize);

	return 0;
}

void A2SServer::Think()
{
	if(!worker)
	{
		return;
	}

	if(!udp)
	{
		printf("A2SServer Initializing UDP\n");
		udp = new leynet_udp;
		udp->Start();


		if(worker->customip.length()>0)
		{
			udp->OpenSocket(worker->a2sport, worker->customip.c_str());
		}else{
			udp->OpenSocket(worker->a2sport);
		}
		
		
		for(uint32_t i=0;i<A2S_MASTERSERVERCOUNT;i++)
		{
			imasterservs[i] = udp->IPStrToInt(masterservs[i]);
			udp->SendTo(imasterservs[i], 27011, "\x71", 1);
			printf("Deleting a2sserv for: %i\n", (int)i);
		}
	
		
		if(worker->sharedport)
		{
			sameportproxy = new ProxyServer(worker);
			printf("Initializing shared Proxy UDP\n");
			sameportproxy->udp = udp;
		}
		
		udp->SetNonBlocking(true);
		udp->SetIgnoreChecksum(true);
		
		this->justinited = 2;
		this->RequestInfoDataIfNeeded();
		
		return;
	}

	this->RequestMasterDataIfNeeded();
	this->RequestInfoDataIfNeeded();
	this->RequestPlayerDataIfNeeded();

	char*worked = udp->Receive(&this->recvsize, &this->recvport, this->recvip, this->a2srecvdata, A2S_MAXDATASIZE);
	//requestwait -= 1;
	
	if (!this->recvsize || this->recvsize > 30000)
	{
		usleep(500);
		//this->OnReceiveNoData();
	}else{
		//requestwait = 10;
		this->OnReceiveData();
		
	}
	


}
