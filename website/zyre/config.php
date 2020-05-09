<?php

function ConnectMysql()
{
    $sqlip = "127.0.0.1";
    $sqluser = "username";
    $sqlpass = "password";
    $sqldb = "zyre";
    $sqlport = 3306;

    $GLOBALS["db"] = new database($sqlip, $sqluser, $sqlpass, $sqldb, $sqlport); 
}


?>
