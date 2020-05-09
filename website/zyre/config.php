<?php

function ConnectMysql()
{
    $sqlip = "127.0.0.1";
    $sqluser = "zyre";
    $sqlpass = "GEJu34\$AWDSFGH";
    $sqldb = "zyre";
    $sqlport = 3306;

    $GLOBALS["db"] = new database($sqlip, $sqluser, $sqlpass, $sqldb, $sqlport); 
}


?>