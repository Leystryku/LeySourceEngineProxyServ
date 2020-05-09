<?php
ini_set('display_errors', 1);

define('LOADED_GLOBALS', 1);

if(!defined('THIS_SCRIPT'))
{
	define('THIS_SCRIPT', '');
}

function autoload_classes($class_name)
{
    $file = 'classes/' . $class_name . '.class.php';
    if (file_exists($file)) require_once $file;
}


spl_autoload_register('autoload_classes');

function getvisitorip()
{
    $ipaddress = '';
    if (getenv('HTTP_CLIENT_IP'))
        $ipaddress = getenv('HTTP_CLIENT_IP');
    else if(getenv('HTTP_X_FORWARDED_FOR'))
        $ipaddress = getenv('HTTP_X_FORWARDED_FOR');
    else if(getenv('HTTP_X_FORWARDED'))
        $ipaddress = getenv('HTTP_X_FORWARDED');
    else if(getenv('HTTP_FORWARDED_FOR'))
        $ipaddress = getenv('HTTP_FORWARDED_FOR');
    else if(getenv('HTTP_FORWARDED'))
        $ipaddress = getenv('HTTP_FORWARDED');
    else if(getenv('REMOTE_ADDR'))
        $ipaddress = getenv('REMOTE_ADDR');
    else
        $ipaddress = 'UNKNOWN';
 
    return $ipaddress;
}


autoload_classes("mysqlidb");

require_once "config.php";

ConnectMysql();

?>