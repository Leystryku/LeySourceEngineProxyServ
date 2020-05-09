<?php
define("IN_LEYPROXYMAN", 1);
define("THIS_SCRIPT", "autoupdate.php");

require_once "global.php";

if(empty($_GET))
{
	echo("err: No get parameters set!");
	return;
}

$ourip = getvisitorip();

$gsport = isset($_GET['gsport']) ? $_GET['gsport'] : false;
$currentversion = isset($_GET['currentversion']) ? $_GET['currentversion'] : '';

$checkver = isset($_GET['checkver']) ? true : false;
$changelog = isset($_GET['changelog']) ? true : false;


if(strlen($currentversion) == 0)
{
	echo("err: No entry found!");
	return;
}

if(!is_numeric($currentversion))
{
	echo("err: Fuck off");
	return;
}

$currentversion = (int)$currentversion;

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

$isnode = false;

if(!$gsport)
{
	$isnode = true;
}

//echo("IP: " . $ourip . ":" . $ourport . " <br>");

$updates = new update($isnode, $currentversion, $ourip, $gsport); 

if(!$updates->HasAccess())
{
	echo("err: no access");
	return;
}

if($checkver)
{
	$outdated = $updates->IsOudated();

	if(!$outdated)
	{
		echo("suc: up to date");
		return;
	}

	echo("suc: " . $outdated);

	return;
}


$data = $updates->GetUpdateIfNeeded($changelog);

if(!$changelog)
{
	header("Content-Type: application/octet-stream");
	header("Content-Transfer-Encoding: Binary");
	header("Content-disposition: attachment; filename=\"data.dat\""); 
}

echo "suc: " . $data;



?>