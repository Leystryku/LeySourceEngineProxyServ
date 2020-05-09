# LeySourceEngineProxyServ
 Ley Proxy Servers for Source engine servers - Zyre.
 Think of cloudflare but for source engine and with geo-cast based server ping enhancing for free.
 ### I've created this tool to
 - A. protect my servers by hiding their real IP entirely. The only visible IPs will be the node IPs.
 - B. To artifically enhance the ping on the server list of source servers to get more players to join.
  Unlike anycast this does not have any costs besides your server costs.
   It avoids duplicates by only letting the location geographically closest (latitude, longtitude)  respond to the user.

 This could potencially be enhanced by using e.g. automatic detection of nodes being down/up.
 Perhaps even automatic IP switching combined with GSLT to basically make your servers non ddosable.


 ### Pros of using this
 - Real server IP becomes invisible, morons trying to DDoS will only attack your nodes and not your real server.
 - You get more players.

 ### Cons of using this
 - Joining over friends list will not work anymore unless you and your friend are in the same location geographically. This could be fixed by figuring out how steam announces the IP on friends list and broadcasting the IP to a non-masterserver registered server, preferabily geographically close to your actual server.
 - Your server ends up having multiple IPs instead of 1. This might confuse people. I generally used to recommend people to just to join over direct IP to closest server using connect mainnodeip.mydomain.net in console
 
 
 ### Project structure
 - geolite/... contains the open source C++ API for maxminds geoip db by 
 - leyzyre/... contains the launcher which launches the nodes for all your servers
 - leyzyremodule/... contains the C++ module which patches the games code to make the proxying possible and fixes up the clients IPs. Also hides real server IP by blocking communication to valve.
 - leyzyrenoderworker/... contains the code behind the nodes. Each node starts 2 servers, one for A2S communication (server info etc.) and one for game traffic (non oob- communication)
 - website/... contains the backend for coordinating the nodes. One node is the master node. Master node basically is automatically selected when we don't have a location in our database for a specific IP.
 sql/... contains the SQL DB for the website
 
 ### Prequisites
 - libmaxminddb
 - curl

 ### Somewhat setup guide
 Step 1.
 Setup a website for coordinating the nodes. Code for the website is in website/ and sql in sql/.
 Step 2.
 Compile leyzyremodule, change the URL to yours and make sure it works for your game. I've used this exclusively for Garry's Mod so certain string references or signatures ("list of opcodes") might not be the same in other games.
 Then proceed to use LD_PRELOAD to load the module.
 Step 3.
 Compile leyzyre and leyzyrenodeworker, change the URL to yours and put them both in the same directory.
 Step 4.
 Get yourself the maxmind geoIP database. I recommend using their autoupdater. Put it in the same dir with the correct file name.
 Step 5.
 Execute and enjoy~
 
 One thing which might be a little bit confusing is the SQL database. There's not a UI to insert your servers etc. in the DB since this wasn't 
 intended for commercial use. I recommend using a tool like NavCat to connect to your SQL db and insert the data there.

### Side notes
Please when using this give credit and only use it for good. I'm releasing this to benefit everyone. I've seen in the past that some people use my open source contributions without giving credit or for bad which quite frankly sucks.
 
 
