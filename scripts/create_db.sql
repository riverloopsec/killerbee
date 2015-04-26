SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL';

CREATE SCHEMA IF NOT EXISTS `ZigBeeSecurity` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci ;
USE `ZigBeeSecurity` ;

-- -----------------------------------------------------
-- Table `ZigBeeSecurity`.`datasources`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `ZigBeeSecurity`.`datasources` ;

CREATE  TABLE IF NOT EXISTS `ZigBeeSecurity`.`datasources` (
  `ds_id` INT NOT NULL AUTO_INCREMENT ,
  `ds_name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`ds_id`) )
ENGINE = InnoDB
COMMENT = 'Devices and programs that provide input data';


-- -----------------------------------------------------
-- Table `ZigBeeSecurity`.`devices`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `ZigBeeSecurity`.`devices` ;

CREATE  TABLE IF NOT EXISTS `ZigBeeSecurity`.`devices` (
  `dev_id` INT NOT NULL AUTO_INCREMENT ,
  `short_addr` CHAR(4) NULL ,
  `long_addr` VARCHAR(16) NULL ,
  `pan_id` CHAR(4) NULL ,
  PRIMARY KEY (`dev_id`) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `ZigBeeSecurity`.`locations`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `ZigBeeSecurity`.`locations` ;

CREATE  TABLE IF NOT EXISTS `ZigBeeSecurity`.`locations` (
  `loc_id` INT NOT NULL AUTO_INCREMENT ,
  `longitude` MEDIUMTEXT  NULL ,
  `latitude` MEDIUMTEXT  NULL ,
  `elevation` MEDIUMTEXT  NULL ,
  PRIMARY KEY (`loc_id`) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `ZigBeeSecurity`.`packets`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `ZigBeeSecurity`.`packets` ;

CREATE  TABLE IF NOT EXISTS `ZigBeeSecurity`.`packets` (
  `pkt_id` INT NOT NULL AUTO_INCREMENT ,
  `db_datetime` DATETIME NOT NULL ,
  `cap_datetime` DATETIME NULL ,
  `ds_id` INT NOT NULL ,
  `rssi` INT NULL ,
  `loc_id` INT NULL ,
  `direction` TINYINT(4)  NULL ,
  `source` INT NULL ,
  `dest` INT NULL ,
  `fcf_frametype` TINYINT(4)  NULL ,
  `fcf_security` TINYINT(1)  NULL ,
  `fcf_pending` TINYINT(1)  NULL ,
  `fcf_ackreq` TINYINT(1)  NULL ,
  `fcf_panidcompress` TINYINT(1)  NULL ,
  `fcf_destaddrmode` TINYINT(4)  NULL ,
  `fcf_srcaddrmode` TINYINT(4)  NULL ,
  `fcf_framever` TINYINT(1)  NULL ,
  `seqnum` TINYINT UNSIGNED NULL ,
  `payload` INT NULL ,
  `packetbytes` VARBINARY(129) NULL ,
  `payloadbytes` VARBINARY(116) NULL ,
  `fcs` CHAR(4) NULL ,
  `channel` SMALLINT NULL ,
  PRIMARY KEY (`pkt_id`) ,
  INDEX `fk_packets_dsid` (`ds_id` ASC) ,
  INDEX `fk_packets_src` (`source` ASC) ,
  INDEX `fk_packets_dest` (`dest` ASC) ,
  INDEX `fk_packets_locid` (`loc_id` ASC) ,
  CONSTRAINT `fk_packets_dsid`
    FOREIGN KEY (`ds_id` )
    REFERENCES `ZigBeeSecurity`.`datasources` (`ds_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_packets_src`
    FOREIGN KEY (`source` )
    REFERENCES `ZigBeeSecurity`.`devices` (`dev_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_packets_dest`
    FOREIGN KEY (`dest` )
    REFERENCES `ZigBeeSecurity`.`devices` (`dev_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_packets_locid`
    FOREIGN KEY (`loc_id` )
    REFERENCES `ZigBeeSecurity`.`locations` (`loc_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Placeholder table for view `ZigBeeSecurity`.`pans`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `ZigBeeSecurity`.`pans` (`pan_id` INT);

-- -----------------------------------------------------
-- Placeholder table for view `ZigBeeSecurity`.`links`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `ZigBeeSecurity`.`links` (`cap_datetime` INT, `src_id` INT, `src_short_addr` INT, `src_pan_id` INT, `seqnum` INT, `dest_id` INT, `dest_short_addr` INT, `dest_pan_id` INT, `src_long_addr` INT, `dest_long_addr` INT);

-- -----------------------------------------------------
-- Placeholder table for view `ZigBeeSecurity`.`linkstatus`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `ZigBeeSecurity`.`linkstatus` (`id` INT);

-- -----------------------------------------------------
-- Placeholder table for view `ZigBeeSecurity`.`devstatus`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `ZigBeeSecurity`.`devstatus` (`dev_id` INT, `short_addr` INT, `long_addr` INT, `pan_id` INT, `seqnum` INT, `cap_datetime` INT);

-- -----------------------------------------------------
-- Placeholder table for view `ZigBeeSecurity`.`locationstatus`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `ZigBeeSecurity`.`locationstatus` (`pkt_id` INT, `loc_id` INT, `rssi` INT, `channel` INT, `longitude` INT, `latitude` INT, `elevation` INT, `fcf_frametype` INT, `source` INT, `dest` INT, `seqnum` INT, `cap_datetime` INT);

-- -----------------------------------------------------
-- View `ZigBeeSecurity`.`pans`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `ZigBeeSecurity`.`pans` ;
DROP TABLE IF EXISTS `ZigBeeSecurity`.`pans`;
USE `ZigBeeSecurity`;
CREATE  OR REPLACE VIEW `ZigBeeSecurity`.`pans` AS SELECT pan_id FROM devices WHERE pan_id IS NOT NULL;

-- -----------------------------------------------------
-- View `ZigBeeSecurity`.`links`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `ZigBeeSecurity`.`links` ;
DROP TABLE IF EXISTS `ZigBeeSecurity`.`links`;
USE `ZigBeeSecurity`;
CREATE  OR REPLACE VIEW `ZigBeeSecurity`.`links` AS
SELECT p.cap_datetime, src.dev_id as src_id, src.short_addr as src_short_addr, src.pan_id as src_pan_id, p.seqnum, dest.dev_id as dest_id, dest.short_addr as dest_short_addr, dest.pan_id as dest_pan_id, src.long_addr as src_long_addr, dest.long_addr as dest_long_addr
	FROM packets as p LEFT JOIN devices as src ON p.source = src.dev_id LEFT JOIN devices as dest ON p.dest = dest.dev_id
	ORDER BY cap_datetime DESC;

-- -----------------------------------------------------
-- View `ZigBeeSecurity`.`linkstatus`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `ZigBeeSecurity`.`linkstatus` ;
DROP TABLE IF EXISTS `ZigBeeSecurity`.`linkstatus`;
USE `ZigBeeSecurity`;
CREATE  OR REPLACE VIEW `ZigBeeSecurity`.`linkstatus` AS
SELECT * FROM links GROUP BY src_id, dest_id;

-- -----------------------------------------------------
-- View `ZigBeeSecurity`.`devstatus`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `ZigBeeSecurity`.`devstatus` ;
DROP TABLE IF EXISTS `ZigBeeSecurity`.`devstatus`;
USE `ZigBeeSecurity`;
CREATE  OR REPLACE VIEW `ZigBeeSecurity`.`devstatus` AS
SELECT dev_id, short_addr, long_addr, pan_id, seqnum, cap_datetime
	FROM devices LEFT JOIN packets ON devices.dev_id = packets.source
	ORDER BY cap_datetime DESC;

-- -----------------------------------------------------
-- View `ZigBeeSecurity`.`locationstatus`
-- -----------------------------------------------------
DROP VIEW IF EXISTS `ZigBeeSecurity`.`locationstatus` ;
DROP TABLE IF EXISTS `ZigBeeSecurity`.`locationstatus`;
USE `ZigBeeSecurity`;
CREATE  OR REPLACE VIEW `ZigBeeSecurity`.`locationstatus` AS
SELECT pkt_id, packets.loc_id, rssi, channel, longitude, latitude, elevation, 
	 fcf_frametype, source, dest, seqnum, cap_datetime
FROM packets LEFT JOIN locations ON packets.loc_id = locations.loc_id
ORDER BY cap_datetime DESC;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;

-- -----------------------------------------------------
-- Data for table `ZigBeeSecurity`.`datasources`
-- -----------------------------------------------------
SET AUTOCOMMIT=0;
USE `ZigBeeSecurity`;
INSERT INTO `ZigBeeSecurity`.`datasources` (`ds_id`, `ds_name`) VALUES (NULL, 'Pcap Import');
INSERT INTO `ZigBeeSecurity`.`datasources` (`ds_id`, `ds_name`) VALUES (NULL, 'Wardrive Live');
INSERT INTO `ZigBeeSecurity`.`datasources` (`ds_id`, `ds_name`) VALUES (NULL, 'zbplant Import');
INSERT INTO `ZigBeeSecurity`.`datasources` (`ds_id`, `ds_name`) VALUES (NULL, 'zbplant Live');
INSERT INTO `ZigBeeSecurity`.`datasources` (`ds_id`, `ds_name`) VALUES (NULL, 'Capture Live');

COMMIT;
