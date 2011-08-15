#ifndef PACKETS_H
#define PACKETS_H

//////////////////////////////////////////////
/////////////////////IMPORTANT////////////////
//////////////////////////////////////////////
#define CRYPT_KEY "w5mOQZ7rHP7pxUSpVc4gOBV8jpx0bpWavByjYWWi2d3lpNgrkf5Vzw1WxDxC09V"
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////

#define SYSTEM_VERSION 3

#define PACKET_SC_WRONG_PACKET 0

//Login
#define PACKET_SC_HELLO 1

//Login
#define PACKET_CS_LOGIN 10
#define PACKET_SC_LOGIN_SUCCESS 10
#define PACKET_SC_LOGIN_FAIL 11
#define PACKET_SC_LOGIN_BANNED 12

//First Info
#define PACKET_CS_GET_FIRST_DATA 20
#define PACKET_SC_GET_FIRST_DATA 20

//Serverinfo
#define PACKET_CS_GET_SERVERINFO 30
#define PACKET_SC_GET_SERVERINFO 30
#define PACKET_SC_GET_SERVERINFO_FAIL 31

//Server Settings
#define PACKET_CS_GET_SERVERSETTINGS 40
#define PACKET_SC_GET_SERVERSETTINGS 40
#define PACKET_SC_GET_SERVERSETTINGS_FAIL 41

#define PACKET_CS_CHANGE_SERVERSETTINGS 50
#define PACKET_SC_CHANGE_SERVERSETTINGS 50
#define PACKET_SC_CHANGE_SERVERSETTINGS_FAIL 51

//Serverstatus-Update
#define PACKET_CS_GET_STATUS 60
#define PACKET_SC_GET_STATUS 60

//Serveractions
#define PACKET_CS_START_SERVER 70
#define PACKET_CS_STOP_SERVER 71
#define PACKET_CS_RESTART_SERVER 72

/*//Server-Logs
#define PACKET_CS_GET_LOGS 80
#define PACKET_SC_GET_LOGS 80*/

//Change Password
#define PACKET_CS_CHANGE_PASSWORD 90
#define PACKET_SC_CHANGE_PASSWORD 90
#define PACKET_SC_CHANGE_PASSWORD_WRONG 91

//LOGOUT
#define PACKET_CS_LOGOUT 100
#define PACKET_SC_LOGOUT 100

//RCON
#define PACKET_CS_RCON 110
#define PACKET_SC_RCON 110

//News
#define PACKET_CS_GET_NEWS 120
#define PACKET_SC_GET_NEWS 120


//Server
#define PACKET_SC_NEW_SERVER 2010
#define PACKET_SC_CHANGE_SERVER 2020
#define PACKET_SC_DELETE_SERVER 2030
//User
#define PACKET_SC_NEW_USER 2040
#define PACKET_SC_CHANGE_USER 2050
#define PACKET_SC_CHANGE_USER_NOW_ADMIN 2051
#define PACKET_SC_DELETE_USER 2060

//News
#define PACKET_SC_NEW_NEWS 2070
#define PACKET_SC_CHANGE_NEWS 2080
#define PACKET_SC_DELETE_NEWS 2090




//////////Admin-Packets//////////
//---Client -> Server---//
#define PACKET_CS_ADMIN_GET_SERVERINFO 1010
#define PACKET_CS_ADMIN_ADD_SERVER 1020
#define PACKET_CS_ADMIN_CHANGE_SERVER 1030
#define PACKET_CS_ADMIN_DELETE_SERVER 1040

#define PACKET_CS_ADMIN_GET_USERINFO 1050
#define PACKET_CS_ADMIN_ADD_USER 1060
#define PACKET_CS_ADMIN_CHANGE_USER 1070
#define PACKET_CS_ADMIN_DELETE_USER 1080

#define PACKET_CS_ADMIN_GET_NEWS 1090
#define PACKET_CS_ADMIN_ADD_NEWS 1100
#define PACKET_CS_ADMIN_CHANGE_NEWS 1110
#define PACKET_CS_ADMIN_DELETE_NEWS 1120

//---Server -> Client---//
//Server
#define PACKET_SC_ADMIN_GET_SERVERINFO 1010
#define PACKET_SC_ADMIN_ADD_SERVER 1020
#define PACKET_SC_ADMIN_ADD_SERVER_FAIL_OWNER 1021
#define PACKET_SC_ADMIN_ADD_SERVER_FAIL_PORT 1022
#define PACKET_SC_ADMIN_ADD_SERVER_FAIL_MAXPLAYER 1023
#define PACKET_SC_ADMIN_ADD_SERVER_FAIL_MAXNPC 1024
#define PACKET_SC_ADMIN_ADD_SERVER_FAIL_PORT_EXISTS 1025

#define PACKET_SC_ADMIN_CHANGE_SERVER 1030
#define PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_OWNER 1031
#define PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_PORT 1032
#define PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_MAXPLAYER 1033
#define PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_MAXNPC 1034
#define PACKET_SC_ADMIN_CHANGE_SERVER_FAIL_PORT_EXISTS 1035
#define PACKET_SC_ADMIN_DELETE_SERVER 1040
#define PACKET_SC_ADMIN_DELETE_SERVER_FAIL 1041
//User
#define PACKET_SC_ADMIN_GET_USERINFO 1050
#define PACKET_SC_ADMIN_GET_USERINFO_FAIL 1051
#define PACKET_SC_ADMIN_ADD_USER 1060
#define PACKET_SC_ADMIN_ADD_USER_FAIL 1061
#define PACKET_SC_ADMIN_CHANGE_USER 1070
#define PACKET_SC_ADMIN_CHANGE_USER_FAIL 1071
#define PACKET_SC_ADMIN_DELETE_USER 1080
#define PACKET_SC_ADMIN_DELETE_USER_FAIL 1081
//News
#define PACKET_SC_ADMIN_GET_NEWS 1090
#define PACKET_SC_ADMIN_GET_NEWS_FAIL 1091
#define PACKET_SC_ADMIN_ADD_NEWS 1100
#define PACKET_SC_ADMIN_ADD_NEWS_FAIL 1101
#define PACKET_SC_ADMIN_CHANGE_NEWS 1110
#define PACKET_SC_ADMIN_CHANGE_NEWS_FAIL 1111
#define PACKET_SC_ADMIN_DELETE_NEWS 1120
#define PACKET_SC_ADMIN_DELETE_NEWS_FAIL 1121


//Filemanager
#define PACKET_CS_FILE_LIST 5000
#define PACKET_CS_FILE_CD 5001
#define PACKET_CS_FILE_BACK 5002
#define PACKET_CS_FILE_CREATE_DIR 5003
#define PACKET_CS_FILE_DELETE 5004


#define PACKET_SC_FILE_LIST 5000
#define PACKET_SC_FILE_CD 5001
#define PACKET_SC_FILE_BACK 5002
#define PACKET_SC_FILE_CREATE_DIR 5003
#define PACKET_SC_FILE_DELETE 5004


#define PACKET_CS_FILE_GET_FILE 5100
#define PACKET_SC_FILE_GET_FILE 5100
#define PACKET_SC_FILE_GET_FILE_FAIL 5101
#define PACKET_CS_FILE_GET_FILE_COMPLETE 5101

#define PACKET_CS_FILE_GET_FILE_PART 5110
#define PACKET_SC_FILE_GET_FILE_PART 5110
#define PACKET_SC_FILE_GET_FILE_PART_FAIL 5111


#define PACKET_CS_FILE_PUT_FILE 5200
#define PACKET_SC_FILE_PUT_FILE 5200
#define PACKET_SC_FILE_PUT_FILE_FAIL 5201
#define PACKET_CS_FILE_PUT_FILE_COMPLETE 5201
#define PACKET_CS_FILE_PUT_FILE_CANCEL 5202
#define PACKET_SC_FILE_PUT_FILE_COMPLETE 5202

#define PACKET_CS_FILE_PUT_FILE_PART 5210
#define PACKET_SC_FILE_PUT_FILE_PART 5210
#define PACKET_SC_FILE_PUT_FILE_PART_FAIL 5211


#endif // PACKETS_H