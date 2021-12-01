-- insert new delay notice messages for server shutdown commands
DELETE FROM `server_string` WHERE `entry` IN (11017,11018);
INSERT INTO `server_string` (`entry`,`content_default`) VALUES
(11017,"Server shutdown delayed to %d seconds as other users are still connected. Specify 'force' to override."),
(11018,"Server shutdown scheduled for T+%d seconds was successfully cancelled.");
