DELETE FROM `server_string` WHERE  `entry`=854 LIMIT 1;
INSERT INTO `server_string` (`entry`, `content_default`, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8) VALUES 
(854, '└ Mails: %d Read/%u Total', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

DELETE FROM `server_string` WHERE  `entry`=871 LIMIT 1;
INSERT INTO `server_string` (`entry`, `content_default`, content_loc1, content_loc2, content_loc3, content_loc4, content_loc5, content_loc6, content_loc7, content_loc8) VALUES 
(871, '│ Level: %u', NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

UPDATE `server_string` SET `content_default`='│ Level: %u (%u/%u XP (%u XP left))' WHERE  `entry`=843 LIMIT 1;
