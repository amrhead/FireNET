/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

Description: Class for creating, sending / reading packets

-------------------------------------------------------------------------
History:

- 20.08.2014   18:29 : Created by AfroStalin(chernecoff)
- 18.03.2015   11:34 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#ifndef _RSP_
#define _RSP_

#include <WinSock.h>

// This define need for checking packet integrity
#define EndBlock "END_BLOCK"

// Packet types
enum EPacketType
{
	PACKET_IDENTIFICATION = 0,
	PACKET_LOGIN, 
	PACKET_REGISTER,
	PACKET_ACCOUNT,
	PACKET_MESSAGE,
	PACKET_REQUEST,
	PACKET_MS_INFO,
	PACKET_GAME_SERVER,
};

// Chat message area. Global, private, system, etc. messages
enum EChatMessageArea
{
	CHAT_MESSAGE_GLOBAL = 0,
	CHAT_MESSAGE_PRIVATE,
	CHAT_MESSAGE_SYSTEM,
};


// Chat message structure. Using for sending chat messages to client/master server
struct SMessage
{
	const char* message;
	EChatMessageArea area; // Global, private, system, etc.
};

// Request packet structure. Using for sending request to client/master server.
struct SRequestPacket
{
	const char* request;
	const char* sParam;
	int iParam;
};

// Login packet structure. Using in login and register client!
struct SLoginPacket
{
	const char* login;
	const char* password;
	const char* nickname;
};

// Game server structur. Using for sending game server info to client/master server.
struct SGameServer
{
	SOCKET socket;
	const char* ip;
	int port;
	const char* serverName;
	int currentPlayers;
	int maxPlayers;
	std::string mapName;
	std::string gameRules;
};

// Player structure
struct SPlayer
{
	int playerID;
	std::string nickName;
	int level;
	int gameCash;
	int realCash;
	int banStatus;
};

// Master server info structure
struct SMasterServerInfo
{
	int playersOnline;
	int gameServersOnline;
};

// Packet structurs
struct SPacket
{
	char *data;
	int size;
	SOCKET addr;
};

// Read/send packet class. Using for sendind/reading packets
class CReadSendPacket
{
public:
	CReadSendPacket(void){}
	~CReadSendPacket(void){}

	// Определяет тип пакета 
	// Gets packet type
	EPacketType GetPacketType (SPacket packet);


	// Отсылает идентификационный пакет
	// Sends identification packet
	void SendIdentificationPacket (SOCKET Socket);

	// Отсылает сообщение 
	// Sends message
	void SendMsg(SOCKET Socket, SMessage message);

#if defined _SERVER
	// Отсылает информацию о игроке (Ник, уровень , кол-во игровой валюты , кол-во валюты за реальные деньги)
	// Sends information about player (Nick, level, number of game currency, count of currency for real money)
	void SendAccountInfo(SOCKET Socket, SPlayer player);

	// Отсылает информацию о мастер сервере
	// Sends information about the master server
	void SendMasterServerInfo (SOCKET Socket, SMasterServerInfo info);
#endif

	// Отсылает информацию о игровом сервере
	// Sends information about the game server
	void SendGameServerInfo (SOCKET Socket, SGameServer server);

	// Отсылает запрос
	// Sends request
	void SendRequest(SOCKET  Socket, SRequestPacket request);

#if defined _CLIENT
	void SendLoginPacket(SOCKET Socket, SLoginPacket packet);
	void SendRegisterPacket(SOCKET Socket, SLoginPacket packet);
#endif

	// Читает идентификационный пакет
	// Read identification packet
	void ReadIdentificationPacket (SPacket packet);

#if defined _SERVER
	// Читает пакет для авторизации
	// Read login packet
	SLoginPacket ReadLoginPacket (SPacket packet);

	// Читает пакет для регистрации
	// Read registration packet
	SLoginPacket ReadRegistrationPacket (SPacket packet);
#endif

	// Читает пакет с сообщением
	// Read message packet
	SMessage ReadMsg(SPacket packet);

#if defined _CLIENT
	// Читает пакет с информацией о игроке
	// Read account information
	void ReadAccountInfo(SPacket packet);
#endif

	// Читает запрос от клиента
	// Read request
	SRequestPacket ReadRequest(SPacket packet);

	// Читает пакет с информацией о игровом сервере
	// Read game server info
	SGameServer ReadGameServerInfo(SPacket packet);
};

#endif