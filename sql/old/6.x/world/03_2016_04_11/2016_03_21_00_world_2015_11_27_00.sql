DELETE FROM `server_string` WHERE `entry` = 11010;
INSERT INTO `server_string` (`entry`,`content_default`) VALUES
(11010, 'You didn\'t get kicked out of the instance even if Player::CheckInstanceLoginValid() returned false and without .gm on flag');
