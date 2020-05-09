<?php

require_once "global.php";

class ServerData
{

	private $ournode = [];
	private $serversforournode = [];
	private $countriesfornode = [];

	private $nodeid = 0;

	private $fndanything = false;

	private $serverpass = false;

	public function HasAccess()
	{
		if($this->fndanything)
			return true;

		return false;
	}

	public function __construct($requesterip, $gsport)
	{
		$db = $GLOBALS["db"];


		$db->con->where("ip", ip2long($requesterip));

		$found = 0;

		if($gsport)
		{
			$found = $db->con->getValue("servers", "count(*)");
		}else{
			$found = $db->con->getValue("nodes", "count(*)");
		}
		

		if(1>$found)
		{
			return;
		}

		if($gsport)
		{
			$db->con->where("port", $gsport);
			$db->con->where("ip", ip2long($requesterip));
			$server = $db->con->getOne("servers");

			if(!$server || !$server['password'])
			{
				echo("no bug");
				return;
			}


			$this->serverpass = $server['password'];
			$this->fndanything = true;
			return;
		}





		$db->con->where("ip", ip2long($requesterip));
		$this->ournode = $db->con->getOne("nodes");

		if(!$this->ournode || !$this->ournode['nodeid'])
		{
			return;
		}	

		$nodeid = $this->ournode['nodeid'];

		$db->con->where("nodeid", $nodeid);
		$this->serversforournode = $db->con->get("servernodecfg");

		if(!$this->serversforournode)
		{
			$this->fndanything = true;
			return;
		}

		foreach ($this->serversforournode as $querykey => $queryresult)
		{
			if(strcmp($queryresult['key'], "activated") || strcmp($queryresult['value'], "1"))
			{
				continue;
			}

			$db->con->where("serverid",$queryresult['serverid'] );
			$relatedserver = $db->con->getOne("servers");

			if(!$relatedserver)
			{
				$this->serversforournode[$querykey]['value'] = "0";
				continue;
			}
			
			$this->serversforournode[$querykey]['value'] = $relatedserver['ip'] . ':' . $relatedserver['port'] . ':' . $relatedserver['password'];
			




		}

		$nodebit = intval($nodeid) -1;

		$this->countriesfornode = $db->con->rawQuery("SELECT * FROM `zyre_countries` WHERE nodebits & 1<<" . $nodebit . " = 1<<" . $nodebit );

		if(!$this->countriesfornode)
		{
			$this->fndanything = false;
			return;
		}

		$this->fndanything = true;

	}

	public function DumpInformation()
	{
		if($this->serverpass)
		{
			echo("suc: " . $this->serverpass);
			return;
		}

		echo("suc: ");

		foreach ($this->countriesfornode as $setting)
		{

			echo(";");
			foreach ($setting as $key => $data)
			{
	    		echo($key . "=" . $data . "<br>");
			}

		}
		echo("?");

		foreach ($this->serversforournode as $setting)
		{
			
			echo(";");
			foreach ($setting as $key => $data)
			{
	    		echo($key . "=" . $data . "<br>");
			}

		}

	}

}

?>