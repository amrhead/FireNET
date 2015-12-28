/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2015
-------------------------------------------------------------------------
-------------------------------------------------------------------------
History:

- 13.03.2015   15:28 : Created by AfroStalin(chernecoff)
- 15.03.2015   23:44 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "Global.h"
#include "winsock.h"

SOCKADDR_IN addr;
SOCKET sConnect; 

int InitWinSock ()
{
	WSAData wsaData ;
	WORD DllVersion = MAKEWORD(2,1); 
	int Val = WSAStartup(DllVersion,&wsaData);

	return Val;
}

void ClientThread(SOCKET ServerSocket)
{
	printf("ClientThread()\n");

	char *Buffer = new char [256];
	int size = 0;

	SMessage Message;
	Message.area = CHAT_MESSAGE_GLOBAL;
	Message.message = "stress message!";

	while(size != SOCKET_ERROR)
	{
		gClientEnv->pRsp->SendMsg(sConnect,Message);

		Sleep(333);
	}

	printf("Server disconnected!\n");
	WSACleanup();
}

int main(int argc, char *argv[])
{
	gClientEnv->Init();
	gClientEnv->pPacketQueue->Init();
	if(InitWinSock() != 0)
	{
		printf("Error startup WinSock!!!\n");
		return 0;
	}

	sConnect = socket(AF_INET, SOCK_STREAM, NULL);


	addr.sin_addr.s_addr = inet_addr ("127.0.0.1");
	addr.sin_port        = htons (64087); 
	addr.sin_family      = AF_INET;

	if(connect (sConnect,(SOCKADDR*)&addr ,sizeof(addr)) !=0)
	{
		printf("Server not available !\n");
		WSACleanup();
	}
	else
	{
		int size = 0 ;
		char *Buffer = new char [512];

		if((size = recv (sConnect,Buffer,256,NULL)) > 0)
		{
			SPacket Packet;
			Packet.data = Buffer;
			Packet.size = size;

			EPacketType PacketType = gClientEnv->pRsp->GetPacketType(Packet);

			switch (PacketType)
			{
			case PACKET_IDENTIFICATION:
				{
					gClientEnv->pRsp->SendIdentificationPacket(sConnect);

					printf("Connection is established\n");

					std::thread clientThread(ClientThread, sConnect);
					clientThread.join();

					break;
				}
			default:
				{
					printf("Connection refused !\n");
					WSACleanup();
					break;
				}
			}
		}
	}

	system("pause");
	return 0;
}
