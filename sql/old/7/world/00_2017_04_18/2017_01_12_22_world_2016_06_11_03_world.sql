-- fix string in server_string incorrectly implying character customization is forced at login
UPDATE `server_string` SET `content_default`='customization for player %s will be requested at next login.' WHERE `entry`=345;
UPDATE `server_string` SET `content_default`='customization for player %s (GUID #%u) will be requested at next login.' WHERE `entry`=346;

UPDATE `server_string` SET `content_default`='You already have a pet.' WHERE `entry`=344; -- add missing article
