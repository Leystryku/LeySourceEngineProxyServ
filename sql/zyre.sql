CREATE TABLE IF NOT EXISTS `zyre_updatesnode` (

  `updateid`  INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
  `changelog` MEDIUMTEXT NOT NULL,
  `data` LONGBLOB NOT NULL


) ENGINE=InnoDB DEFAULT CHARSET=utf8 DEFAULT COLLATE utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `zyre_updatesmodule` (

  `updateid`  INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
  `changelog` MEDIUMTEXT NOT NULL,
  `data` LONGBLOB NOT NULL


) ENGINE=InnoDB DEFAULT CHARSET=utf8 DEFAULT COLLATE utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `zyre_nodes` (
  `nodeid` TINYINT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
  `ip` INT UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 DEFAULT COLLATE utf8_unicode_ci;

CREATE UNIQUE INDEX zyre_nodes_ipdup ON zyre_nodes(`ip`);


CREATE TABLE IF NOT EXISTS `zyre_nodesextraips` (
  `nodeid` TINYINT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
  `extraip` INT UNSIGNED NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 DEFAULT COLLATE utf8_unicode_ci;

CREATE UNIQUE INDEX zyre_nodesextraips_nodup ON zyre_nodesextraips(`extraip`);

CREATE TABLE IF NOT EXISTS `zyre_countries` (
  `countryid`  INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
  `nodebits` BIGINT UNSIGNED NOT NULL,
  `countryname` varchar(64) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 DEFAULT COLLATE utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `zyre_servers` (
  `serverid`  INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
  `ip` INT UNSIGNED NOT NULL,
  `port` SMALLINT UNSIGNED NOT NULL,
  `password` BIGINT NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8 DEFAULT COLLATE utf8_unicode_ci;

CREATE UNIQUE INDEX zyre_servers_index_ip_and_port ON zyre_servers(`ip`, `port`);

CREATE TABLE IF NOT EXISTS `zyre_servernodecfg` (
  `cfgid` INT UNSIGNED AUTO_INCREMENT NOT NULL PRIMARY KEY,
  `serverid`  INT UNSIGNED NOT NULL,
  `nodeid` TINYINT UNSIGNED NOT NULL,
  `key` varchar(64) NOT NULL,
  `value` varchar(64) NOT NULL,
   FOREIGN KEY  (nodeid) REFERENCES zyre_nodes(nodeid),
   FOREIGN KEY  (serverid) REFERENCES zyre_servers (serverid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 DEFAULT COLLATE utf8_unicode_ci;

INSERT INTO `zyre_countries`(`nodebits`, `countryname`) VALUES (0xFFFFFFFFFFFFFFFF, 'All Countries');
INSERT INTO `zyre_countries`(`nodebits`, `countryname`) VALUES (0xFFFFFFFFFFFFFFFF, 'Unidentified Countries');

INSERT INTO `zyre_nodes`(`ip`) VALUES (INET_ATON("127.0.0.1"));
INSERT INTO `zyre_servers`(`ip`, `port`, `password`) VALUES (INET_ATON("127.0.0.1"), 27015, UNIX_TIMESTAMP());

INSERT INTO `zyre_servernodecfg`(`serverid`, `nodeid`, `key`, `value`) VALUES (1,1,'activated', '1');
INSERT INTO `zyre_servernodecfg`(`serverid`, `nodeid`, `key`, `value`) VALUES (1,1,'proxyport', '27015');