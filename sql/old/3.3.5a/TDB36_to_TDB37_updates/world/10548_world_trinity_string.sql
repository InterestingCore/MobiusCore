DELETE FROM `server_string` WHERE `entry` IN (5022, 5023);
INSERT INTO `server_string` (`entry`, `content_default`) VALUES
(5022, 'Granting ownership to first person that joins the channel \"%s\": Enabled.'),
(5023, 'Granting ownership to first person that joins the channel \"%s\": Disabled.');
