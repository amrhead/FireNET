/*************************************************************************
"BeatGames" Source File.
Copyright (C), BeatGames, 2014-2015
-------------------------------------------------------------------------

-------------------------------------------------------------------------
History:

- 14.08.2014   22:09 : Created by AfroStalin(chernecoff)
- 07.03.2015   02:43 : Edited by AfroStalin(chernecoff)
-------------------------------------------------------------------------

*************************************************************************/

#ifndef _Packets_
#define _Packets_

#include "BasePacket.h"

class Packet: public BasePacket
{
public:
	Packet();
	Packet( const unsigned char *bytes, unsigned int length );
	virtual ~Packet();
public:
	virtual bool         decodeBlowfish( bool bUseStaticBFKey );
	virtual bool         decodeBlowfish( unsigned char *blowfishKey );
public:
	virtual bool         appendChecksum( bool append4bytes = true );
	static bool          verifyBytesChecksum( const unsigned char *bytes, unsigned int offset, unsigned int size );
	virtual bool         verifyChecksum() const;
	virtual bool         padPacketTo8ByteLen();
	virtual bool         appendMore8Bytes();
	virtual bool         setDynamicBFKey( unsigned char *newKey, unsigned int newKeyLen );
	virtual bool         encodeBlowfish( bool bUseStaticBFKey );

public:
	virtual bool encodeAndPrepareToSend( unsigned char *blowfishKey, unsigned int bfKeyLen = 16 );

protected:
	virtual void _initNull();
protected:
	unsigned char STATIC_BLOWFISH_KEY[64];
	unsigned int  STATIC_BLOWFISH_KEY_LEN;
	unsigned char NEW_BLOWFISH_KEY[64];
	unsigned int  NEW_BLOWFISH_KEY_LEN;
	unsigned int  xor_key;
};

#endif