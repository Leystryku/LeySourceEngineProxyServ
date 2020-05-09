<?php
class database
{


    public $con;

    public function __construct($host, $username, $password, $dbname, $port)
    {

        $db = MysqliDb::getInstance();

        if(isset($db))
        {
            echo("cached db");

            if (!$db->ping())
                $db->connect();

            if(!$db) die("Database error");

            return $db;
        }

        $db = new MysqliDb (Array (
                'host' => $host,
                'username' => $username, 
                'password' => $password,
                'db'=> $dbname,
                'port' => 3306,
                'prefix' => 'zyre_',
                'charset' => 'utf8'));

        if(!$db) die("Database error");

        $this->con = $db;
        $db->connect();

    }

}

?>