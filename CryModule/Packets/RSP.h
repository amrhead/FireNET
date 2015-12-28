/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2014-2015
-------------------------------------------------------------------------
History:

- 20.08.2014   18:29 : Created by AfroStalin(chernecoff)
- 23.05.2015   00:50  : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------


*************************************************************************/

#ifndef _RSP_
#define _RSP_

typedef UINT_PTR SOCKET;

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
	PACKET_GAME_SERVERS,
	PACKET_CONSOLE_TEXT,
	PACKET_CONSOLE_COMMAND,
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
	int id;
	const char* ip;
	int port;
	const char* serverName;
	int currentPlayers;
	int maxPlayers;
	char* mapName;
	char* gameRules;
};

// Player structure
struct SPlayer
{
	int playerId;
	std::string nickname;
	int level;
	int money;
	int xp;
	bool banStatus;
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


	// Отсылает информацию о игровом сервере
	// Sends information about the game server
	void SendGameServerInfo (SOCKET Socket, SGameServer server);

	// Отсылает запрос
	// Sends request
	void SendRequest(SOCKET  Socket, SRequestPacket request);

	//Send login packet
	void SendLoginPacket(SOCKET Socket, SLoginPacket packet);

	//Send register packet
	void SendRegisterPacket(SOCKET Socket, SLoginPacket packet);

	/* Console helper */
	void SendConsoleTextPacket(SOCKET Socket, int textType, const char* text);

	void ReadConsoleCommandPacket(SPacket packet);
	/*----------------*/

	// Читает идентификационный пакет
	// Read identification packet
	void ReadIdentificationPacket (SPacket packet);

	// Читает пакет с сообщением
	// Read message packet
	SMessage ReadMsg(SPacket packet);

	// Читает пакет с информацией о игроке
	// Read account information
	SPlayer ReadAccountInfo(SPacket packet);

	// Читает запрос от клиента
	// Read request
	SRequestPacket ReadRequest(SPacket packet);

	// Читает пакет с информацией о игровом сервере
	// Read game server info
	SGameServer ReadGameServerInfo(SPacket packet);

	void ReadGameServers(SPacket packet);

	// Read master server info
	SMasterServerInfo ReadMasterServerInfo(SPacket packet);
};

#endif