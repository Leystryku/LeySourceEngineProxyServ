<?php

require_once "global.php";


class Update
{


	private $requestersversion = 0;
	private $latestversion = 0;
	private $isnode = false;

	private $hasaccess = false;

	public function __construct($isnode, $requesterversion, $requesterip, $requesterport)
	{
		if(0>$requesterversion)
		{
			$requesterversion = 0;
		}

		$this->requestersversion = $requesterversion;
		
		$this->isnode = $isnode;


		$db = $GLOBALS["db"];

		$db->con->where ("ip", ip2long($requesterip));

		if($this->isnode)
		{
			$nodes = $db->con->getOne("nodes");

			if(!$nodes)
			{
				//echo("no node");
				return;
			}
			$this->hasaccess = true;

			$db->con->orderBy("updateid","desc");
			$results = $db->con->get('updatesnode', Array (0, 1));

			if(!$results||!$results[0])
			{
				$this->latestversion = 0;
				return;
			}

			$this->latestversion = $results[0]['updateid'];
			return;
		}

		$db->con->where ("port", $requesterport);
		$servers = $db->con->getOne("servers");

		if(!$servers)
		{
			//echo("no server");
			return;
		}


		$this->hasaccess = true;
		
		$db->con->orderBy("updateid","desc");
		$results = $db->con->get('updatesmodule', Array (0, 1));

		if(!$results||!$results[0])
		{
			$this->latestversion = 0;
			return;
		}
		$this->latestversion = $results[0]['updateid'];


	}

	public function HasAccess()
	{
		return $this->hasaccess;
	}

	public function IsOudated()
	{
		if($this->latestversion>$this->requestersversion)
			return $this->latestversion;

		if(0>$this->latestversion)
			return $this->latestversion;

		return false;
	}

	public function GetUpdateIfNeeded($changelog)
	{
		if(!$this->IsOudated())
			return "";

		$db = $GLOBALS["db"];


		$updatedata = 0;

		$db->con->where("updateid", $this->latestversion);
		if($this->isnode)
		{
			$updatedata = $db->con->getOne("updatesnode");
		}else{
			$updatedata = $db->con->getOne("updatesmodule");
		}



		//insert mysql magic here

		$data = $updatedata["changelog"];

		if($changelog == false)
		{
			$data = $updatedata["data"];
		}

		return $data;
	}


}

?>