<?php

define("IN_LEYPROXYMAN", 1);
define("THIS_SCRIPT", "serverdata.php");

require_once "global.php";

$requesterip = getvisitorip();

$gsport = isset($_GET['gsport']) ? $_GET['gsport'] : false;

if($gsport)
{
	if(!is_numeric($gsport))
	{
		echo("err: Fuck off");
		return;
	}

	$gsport = (int)$gsport;

	if($gsport==0)
	{
		echo("err: Fuck off");
		return;
	}
}

$fetchdata = new serverdata($requesterip, $gsport); 

if(!$fetchdata->HasAccess())
{
	echo("err: No access!");
	return;
}

$fetchdata->DumpInformation();

?>