-- --------------------------------------------------------
-- Host:                         IP LUL
-- Server Version:               5.7.24-0ubuntu0.18.04.1 - (Ubuntu)
-- Server Betriebssystem:        Linux
-- HeidiSQL Version:             11.0.0.5919
-- --------------------------------------------------------

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET NAMES utf8 */;
/*!50503 SET NAMES utf8mb4 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;

-- Exportiere Struktur von Tabelle zyre.zyre_nodes
CREATE TABLE IF NOT EXISTS `zyre_nodes` (
  `nodeid` tinyint(3) unsigned NOT NULL AUTO_INCREMENT,
  `ip` int(10) unsigned NOT NULL,
  `name` varchar(20) COLLATE utf8_unicode_ci DEFAULT NULL,
  `lat` decimal(10,8) NOT NULL,
  `lng` decimal(11,8) NOT NULL,
  `ismaster` tinyint(4) DEFAULT NULL,
  PRIMARY KEY (`nodeid`),
  UNIQUE KEY `zyre_nodes_ipdup` (`ip`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Exportiere Daten aus Tabelle zyre.zyre_nodes: ~8 rows (ungefähr)
/*!40000 ALTER TABLE `zyre_nodes` DISABLE KEYS */;
INSERT INTO `zyre_nodes` (`nodeid`, `ip`, `name`, `lat`, `lng`, `ismaster`) VALUES
	(1, 1110079139, 'US_Chicago', 40.46520000, -74.23070000, 0),
	(2, 759969977, 'US_Dallas', 32.79040000, -96.80440000, 0),
	(3, 1605599518, 'NL_Amsterdam', 52.39040000, 4.65630000, 0),
	(4, 1605617065, 'UK_London', 51.51300000, -0.06180000, 1),
	(5, 1760473217, 'DE_Frankfurt', 50.09790000, 8.59990000, 0),
	(6, 2429184044, 'US_SilliconV', 37.33870000, -121.89140000, 0),
	(7, 3354856607, 'FR_Paris', 48.84120000, 2.38760000, 0),
	(8, 760028114, 'JP_Tokyo', 35.58330000, 139.74830000, 0);
/*!40000 ALTER TABLE `zyre_nodes` ENABLE KEYS */;

-- Exportiere Struktur von Tabelle zyre.zyre_nodesextraips
CREATE TABLE IF NOT EXISTS `zyre_nodesextraips` (
  `nodeid` tinyint(3) unsigned NOT NULL AUTO_INCREMENT,
  `extraip` int(10) unsigned NOT NULL,
  PRIMARY KEY (`nodeid`),
  UNIQUE KEY `zyre_nodesextraips_nodup` (`extraip`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Exportiere Daten aus Tabelle zyre.zyre_nodesextraips: ~0 rows (ungefähr)
/*!40000 ALTER TABLE `zyre_nodesextraips` DISABLE KEYS */;
/*!40000 ALTER TABLE `zyre_nodesextraips` ENABLE KEYS */;

-- Exportiere Struktur von Tabelle zyre.zyre_servernodecfg
CREATE TABLE IF NOT EXISTS `zyre_servernodecfg` (
  `cfgid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `serverid` int(10) unsigned NOT NULL,
  `nodeid` tinyint(3) unsigned NOT NULL,
  `key` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  `value` varchar(64) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`cfgid`),
  KEY `nodeid` (`nodeid`),
  KEY `serverid` (`serverid`),
  CONSTRAINT `zyre_servernodecfg_ibfk_1` FOREIGN KEY (`nodeid`) REFERENCES `zyre_nodes` (`nodeid`),
  CONSTRAINT `zyre_servernodecfg_ibfk_2` FOREIGN KEY (`serverid`) REFERENCES `zyre_servers` (`serverid`)
) ENGINE=InnoDB AUTO_INCREMENT=112 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Exportiere Daten aus Tabelle zyre.zyre_servernodecfg: ~96 rows (ungefähr)
/*!40000 ALTER TABLE `zyre_servernodecfg` DISABLE KEYS */;
INSERT INTO `zyre_servernodecfg` (`cfgid`, `serverid`, `nodeid`, `key`, `value`) VALUES
	(1, 1, 1, 'activated', '1'),
	(2, 1, 1, 'a2sport', '27015'),
	(3, 2, 1, 'activated', '1'),
	(4, 2, 1, 'a2sport', '27016'),
	(5, 3, 1, 'activated', '1'),
	(6, 3, 1, 'a2sport', '27017'),
	(7, 1, 2, 'activated', '1'),
	(8, 1, 2, 'a2sport', '27015'),
	(9, 2, 2, 'activated', '1'),
	(10, 2, 2, 'a2sport', '27016'),
	(11, 3, 2, 'activated', '1'),
	(12, 3, 2, 'a2sport', '27017'),
	(13, 1, 3, 'activated', '1'),
	(14, 1, 3, 'a2sport', '27015'),
	(15, 2, 3, 'activated', '1'),
	(16, 2, 3, 'a2sport', '27016'),
	(17, 3, 3, 'activated', '1'),
	(18, 3, 3, 'a2sport', '27017'),
	(19, 1, 5, 'activated', '1'),
	(20, 1, 5, 'a2sport', '27015'),
	(21, 2, 5, 'activated', '1'),
	(22, 2, 5, 'a2sport', '27016'),
	(23, 3, 5, 'activated', '1'),
	(24, 3, 5, 'a2sport', '27017'),
	(25, 1, 4, 'a2sport', '27015'),
	(26, 2, 4, 'activated', '1'),
	(27, 2, 4, 'a2sport', '27016'),
	(28, 3, 4, 'activated', '1'),
	(29, 1, 4, 'activated', '1'),
	(30, 3, 4, 'a2sport', '27017'),
	(31, 1, 1, 'extraplayers', '0'),
	(32, 1, 2, 'extraplayers', '0'),
	(33, 1, 3, 'extraplayers', '0'),
	(34, 1, 4, 'extraplayers', '0'),
	(35, 1, 5, 'extraplayers', '0'),
	(43, 2, 1, 'extraplayers', '0'),
	(44, 2, 2, 'extraplayers', '0'),
	(45, 2, 3, 'extraplayers', '0'),
	(46, 2, 4, 'extraplayers', '0'),
	(47, 2, 5, 'extraplayers', '0'),
	(49, 3, 1, 'extraplayers', '0'),
	(50, 3, 2, 'extraplayers', '0'),
	(51, 3, 3, 'extraplayers', '0'),
	(52, 3, 4, 'extraplayers', '0'),
	(53, 3, 5, 'extraplayers', '0'),
	(55, 1, 6, 'activated', '1'),
	(56, 1, 6, 'a2sport', '27015'),
	(57, 2, 6, 'activated', '1'),
	(58, 2, 6, 'a2sport', '27016'),
	(59, 3, 6, 'activated', '1'),
	(60, 3, 6, 'a2sport', '27017'),
	(67, 1, 7, 'activated', '1'),
	(68, 1, 7, 'a2sport', '27015'),
	(69, 2, 7, 'activated', '1'),
	(70, 2, 7, 'a2sport', '27016'),
	(71, 3, 7, 'activated', '1'),
	(72, 3, 7, 'a2sport', '27017'),
	(73, 1, 6, 'extraplayers', '0'),
	(74, 2, 6, 'extraplayers', '0'),
	(75, 3, 6, 'extraplayers', '0'),
	(76, 1, 7, 'extraplayers', '0'),
	(77, 2, 7, 'extraplayers', '0'),
	(78, 3, 7, 'extraplayers', '0'),
	(79, 1, 8, 'extraplayers', '0'),
	(80, 2, 8, 'extraplayers', '0'),
	(81, 3, 8, 'extraplayers', '0'),
	(82, 1, 8, 'activated', '1'),
	(83, 1, 8, 'a2sport', '27015'),
	(84, 2, 8, 'activated', '1'),
	(85, 2, 8, 'a2sport', '27016'),
	(86, 3, 8, 'activated', '1'),
	(87, 3, 8, 'a2sport', '27017'),
	(88, 4, 1, 'a2sport', '27018'),
	(89, 4, 4, 'a2sport', '27018'),
	(90, 4, 7, 'a2sport', '27018'),
	(91, 4, 4, 'extraplayers', '0'),
	(92, 4, 8, 'extraplayers', '0'),
	(93, 4, 8, 'a2sport', '27018'),
	(94, 4, 5, 'a2sport', '27018'),
	(95, 4, 5, 'activated', '1'),
	(96, 4, 4, 'activated', '1'),
	(97, 4, 3, 'extraplayers', '0'),
	(98, 4, 6, 'a2sport', '27018'),
	(99, 4, 3, 'a2sport', '27018'),
	(100, 4, 3, 'activated', '1'),
	(101, 4, 1, 'extraplayers', '0'),
	(102, 4, 5, 'extraplayers', '0'),
	(103, 4, 6, 'extraplayers', '0'),
	(104, 4, 8, 'activated', '1'),
	(105, 4, 2, 'a2sport', '27018'),
	(106, 4, 2, 'activated', '1'),
	(107, 4, 2, 'extraplayers', '0'),
	(108, 4, 6, 'activated', '1'),
	(109, 4, 7, 'activated', '1'),
	(110, 4, 7, 'extraplayers', '0'),
	(111, 4, 1, 'activated', '1');
/*!40000 ALTER TABLE `zyre_servernodecfg` ENABLE KEYS */;

-- Exportiere Struktur von Tabelle zyre.zyre_servers
CREATE TABLE IF NOT EXISTS `zyre_servers` (
  `serverid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `ip` int(10) unsigned NOT NULL,
  `port` smallint(5) unsigned NOT NULL,
  `password` bigint(20) NOT NULL,
  PRIMARY KEY (`serverid`),
  UNIQUE KEY `zyre_servers_index_ip_and_port` (`ip`,`port`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Exportiere Daten aus Tabelle zyre.zyre_servers: ~4 rows (ungefähr)
/*!40000 ALTER TABLE `zyre_servers` DISABLE KEYS */;
INSERT INTO `zyre_servers` (`serverid`, `ip`, `port`, `password`) VALUES
	(1, 1495425417, 27015, 7522941189),
	(2, 1495425417, 27016, 8522941189),
	(3, 1495425417, 27017, 6522941189),
	(4, 1495425417, 27018, 6522941189);
/*!40000 ALTER TABLE `zyre_servers` ENABLE KEYS */;

-- Exportiere Struktur von Tabelle zyre.zyre_updatesmodule
CREATE TABLE IF NOT EXISTS `zyre_updatesmodule` (
  `updateid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `changelog` mediumtext COLLATE utf8_unicode_ci NOT NULL,
  `data` longblob NOT NULL,
  PRIMARY KEY (`updateid`)
) ENGINE=InnoDB AUTO_INCREMENT=15 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- Exportiere Struktur von Tabelle zyre.zyre_updatesnode
CREATE TABLE IF NOT EXISTS `zyre_updatesnode` (
  `updateid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `changelog` mediumtext COLLATE utf8_unicode_ci NOT NULL,
  `data` longblob NOT NULL,
  PRIMARY KEY (`updateid`)
) ENGINE=InnoDB AUTO_INCREMENT=28 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

/*!40101 SET SQL_MODE=IFNULL(@OLD_SQL_MODE, '') */;
/*!40014 SET FOREIGN_KEY_CHECKS=IF(@OLD_FOREIGN_KEY_CHECKS IS NULL, 1, @OLD_FOREIGN_KEY_CHECKS) */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
