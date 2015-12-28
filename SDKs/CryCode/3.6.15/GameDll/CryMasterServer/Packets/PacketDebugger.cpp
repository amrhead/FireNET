/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

Description: Дебагинг входящих/исходящих пакетов
Description: Debaging incoming / outgoing packets
-------------------------------------------------------------------------
History:

- 17.08.2014   22:50 : Created by AfroStalin(chernecoff)
- 08.03.2015   16:37 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/
#include "StdAfx.h"
#include "PacketDebugger.h"
#include <iostream>
#include <fstream>

namespace PacketDebugger
{
		char* GetHexFromByte(char c)
		{
			char* result = new char[3];
			result[2] = (char)0x00;
		
			char hi = ((char)(c >> 4)) << 4;
			char low = c - hi;
		
			switch(hi)
			{
			case (char)0x00:
				result[0] = '0';
				break;
			case (char)0x10:
				result[0] = '1';
				break;
			case (char)0x20:
				result[0] = '2';
				break;
			case (char)0x30:
				result[0] = '3';
				break;
			case (char)0x40:
				result[0] = '4';
				break;
			case (char)0x50:
				result[0] = '5';
				break;
			case (char)0x60:
				result[0] = '6';
				break;
			case (char)0x70:
				result[0] = '7';
				break;
			case (char)0x80:
				result[0] = '8';
				break;
			case (char)0x90:
				result[0] = '9';
				break;
			case (char)0xA0:
				result[0] = 'A';
				break;
			case (char)0xB0:
				result[0] = 'B';
				break;
			case (char)0xC0:
				result[0] = 'C';
				break;
			case (char)0xD0:
				result[0] = 'D';
				break;
			case (char)0xE0:
				result[0] = 'E';
				break;
			case (char)0xF0:
				result[0] = 'F';
				break;
			default:
				throw "";
			}

			switch(low)
			{
			case (char)0x00:
				result[1] = '0';
				break;
			case (char)0x01:
				result[1] = '1';
				break;
			case (char)0x02:
				result[1] = '2';
				break;
			case (char)0x03:
				result[1] = '3';
				break;
			case (char)0x04:
				result[1] = '4';
				break;
			case (char)0x05:
				result[1] = '5';
				break;
			case (char)0x06:
				result[1] = '6';
				break;
			case (char)0x07:
				result[1] = '7';
				break;
			case (char)0x08:
				result[1] = '8';
				break;
			case (char)0x09:
				result[1] = '9';
				break;
			case (char)0x0A:
				result[1] = 'A';
				break;
			case (char)0x0B:
				result[1] = 'B';
				break;
			case (char)0x0C:
				result[1] = 'C';
				break;
			case (char)0x0D:
				result[1] = 'D';
				break;
			case (char)0x0E:
				result[1] = 'E';
				break;
			case (char)0x0F:
				result[1] = 'F';
				break;
			default:
				throw "";
			}

			return result;
		}

		void Debug(char* buffer, int length, char* filename)
		{
			if(length < 1 || buffer == 0 || filename == 0)
				return;

			std::ofstream log;
			log.open (filename, std::ios::app);
		
			for(int i=0;i<length;i++)
				log << PacketDebugger::GetHexFromByte(buffer[i]) << " ";
		
			log << std::endl;
		
			log.close();		
		}
}
